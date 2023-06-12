#version 460 core

#define MAXIMUM_LIGHTS 5
#define MAXIMUM_TEXTURE_UNITS 5

struct Material 
{
    int nDiffuseTexture;
    int nSpecularTexture;
    sampler2D diffuse[MAXIMUM_TEXTURE_UNITS];
    sampler2D specular[MAXIMUM_TEXTURE_UNITS];
    float shininess;
}; 

struct PointLight 
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuation;
};

struct DirectionalLight 
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight
{
    vec3 position;
    vec3 direction;
    float innerCutOff;
    float outerCutOff;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuation;
};

in vec2 fTextureCoord;
in vec3 fNormal;
in vec3 fragPosition;
out vec4 FragColor;

uniform Material material;
uniform PointLight pointLight[MAXIMUM_LIGHTS];
uniform int numPointLights;
uniform DirectionalLight directionalLight[MAXIMUM_LIGHTS];
uniform int numDirectionalLights;
uniform SpotLight spotLight[MAXIMUM_LIGHTS];
uniform int numSpotLights;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform vec3 viewPosition;


// Function Declarations
void processTextureUnits (out vec3 diffuseColor,out vec3 specularColor );
vec3 CalcPointLight(PointLight light, vec3 diffuseColor, vec3 specularColor);
vec3 CalcDirectionalLight(DirectionalLight light,vec3 diffuseColor, vec3 specularColor); 
vec3 CalcSpotLight(SpotLight light, vec3 diffuseColor, vec3 specularColor);


/*============================================================================================================================================================================================
 * Entry Point
 *============================================================================================================================================================================================*/


void main()
{
    vec3 diffuseColor;
    vec3 specularColor;
    vec3 result = vec3(0.0f, 0.0f, 0.0f);
    processTextureUnits (diffuseColor, specularColor);
    for (int i = 0; i < numPointLights; i++)
    {
        result += CalcPointLight(pointLight[i], diffuseColor, specularColor);
    }
    for (int i = 0; i < numDirectionalLights; i++)
    {
        result += CalcDirectionalLight(directionalLight[i], diffuseColor, specularColor);
    }
    for (int i = 0; i < numSpotLights; i++)
    {
        result+= CalcSpotLight(spotLight[i], diffuseColor, specularColor);
    }
    
    FragColor = vec4(result, 1.0f);
}

/*============================================================================================================================================================================================
 * Process Texture Units
 *============================================================================================================================================================================================*/


void processTextureUnits (out vec3 diffuseColor, out vec3 specularColor )
{
    if (material.nDiffuseTexture == 1)
    {
       diffuseColor = vec3(texture(material.diffuse[0], fTextureCoord));
       specularColor = vec3(texture(material.specular[0], fTextureCoord));
    }
    else 
    {
        vec4 diffuseMix = mix(texture(material.diffuse[0], fTextureCoord), texture(material.diffuse[1], fTextureCoord), 0.2);
        vec4 specularMix = mix(texture(material.specular[0], fTextureCoord), texture(material.specular[1], fTextureCoord), 0.2);
        for (int i = 0; i < material.nSpecularTexture - 2; i++)
        {
            diffuseMix = mix(diffuseMix, texture(material.diffuse[i + 2], fTextureCoord), 0.2);
            specularMix = mix(specularMix, texture(material.specular[i + 2], fTextureCoord), 0.2);
        }
        diffuseColor = vec3(diffuseMix);
        specularColor = vec3(specularMix);
    }
}

/*============================================================================================================================================================================================
 * Calculate Point Light, Directional Light, and Spot Light
 *============================================================================================================================================================================================*/


vec3 CalcPointLight(PointLight light, vec3 diffuseColor, vec3 specularColor)
{
    vec3 ambient = light.ambient * diffuseColor ;

    vec3 norm = normalize(fNormal);
    vec3 lightDir = normalize(light.position - fragPosition);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * diffuseColor);

    vec3 viewDir = normalize(viewPosition - fragPosition);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (specularColor * spec);

    float distance = length(light.position - fragPosition);
    float attenuation = 1.0 / (light.attenuation.x + (light.attenuation.y * distance) + (light.attenuation.z * (distance * distance)));
    ambient  *= attenuation; 
    diffuse  *= attenuation;
    specular *= attenuation; 

    return (ambient + diffuse + specular);
}

vec3 CalcDirectionalLight(DirectionalLight light, vec3 diffuseColor, vec3 specularColor)
{
    vec3 ambient = light.ambient * diffuseColor ;

    vec3 norm = normalize(fNormal);
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * diffuseColor);

    vec3 viewDir = normalize(viewPosition - fragPosition);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (specularColor * spec) ;
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 diffuseColor, vec3 specularColor)
{
    // Regular Light Calculation
    vec3 ambient = light.ambient * diffuseColor;
    vec3 norm = normalize(fNormal);
    vec3 lightDir = normalize(light.position - fragPosition);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * diffuseColor);
    vec3 viewDir = normalize(viewPosition - fragPosition);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (specularColor * spec);
    // Attenuation
    float distance = length(light.position - fragPosition);
    float attenuation = 1.0 / (light.attenuation.x + (light.attenuation.y * distance) + (light.attenuation.z * (distance * distance)));
    ambient  *= attenuation; 
    diffuse  *= attenuation;
    specular *= attenuation; 
    // Spotlight Calculation
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.innerCutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0f, 1.0f);

    diffuse *= intensity;
    specular *= intensity;
    
    
    return (ambient + diffuse + specular);

    
}