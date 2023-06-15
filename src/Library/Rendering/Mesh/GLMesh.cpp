#include "Meshes.h"
#include "../../Library.h"
/*============================================================================================================================================================================================
 *============================================================================================================================================================================================
 * GLMesh Class
 *============================================================================================================================================================================================
 *============================================================================================================================================================================================*/

GLMesh::GLMesh(GLMesh::OUTPUTTYPE outputType, std::vector<unsigned int>& indices, std::vector<Vertex>& vertices)
{
    output = outputType;

    this->indices = indices;
    textures.push_back(Resources::textureManager.crate);
    textures.push_back(Resources::textureManager.crateSpec);
    textures.push_back(Resources::textureManager.smileyFace);
    textures.push_back(Resources::textureManager.smileyFaceSpec);

    this->vertices = vertices;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbos[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

GLMesh::~GLMesh() 
{
    glDeleteVertexArrays(1, &vao);
    if (indices.size() == 0)
    {
        glDeleteBuffers(1, vbos);
    }
    else
    {
        glDeleteBuffers(2, vbos);
    }
    vao = -1;
    for (Texture* texture : textures)
    {
        if (!texture) { continue; }
        glDeleteTextures(1, &texture->id);
    }
    
    int vbosSize = sizeof(vbos) / sizeof(int);
    for (int i = 0; i < vbosSize; i++)
    {
        vbos[i] = -1;
    }
    textures.clear();
    vertices.clear();
    output = GLMesh::NOOUTPUT;
    
}

 /*============================================================================================================================================================================================
  * GLMesh Functionality
  *============================================================================================================================================================================================*/

void GLMesh::setupMesh()
{

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbos[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    if (indices.size() > 0)
    {
        glGenBuffers(1, &vbos[1]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
            &indices[0], GL_STATIC_DRAW);
    }

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}


void GLMesh::Draw(void* objectRenderer)
{
    Renderable* renderer = static_cast<Renderable*>(objectRenderer);
    GLShader* shader = renderer->shader;
    // Matrix Local Variables
    glm::mat4 scale, rotation, translation;
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 projection, view;
    unsigned int diffuseNr = 0;
    unsigned int specularNr = 0;
    unsigned int totalNumber = 0;
    // Attach Appropriate texture units
    shader->useProgram();

    std::string textureName;

    if (shader->type == GLShader::LIGHTSOURCE)
    {
        // Do nothing
    }
    else
    {
        for (Texture* texture : textures)
        {
            if (texture->type == "texture_diffuse")
            {
                textureName = "material.diffuse[" + std::to_string(diffuseNr) + "]";
                shader->setInt(textureName, totalNumber);
                glActiveTexture(GL_TEXTURE0 + totalNumber);
                glBindTexture(GL_TEXTURE_2D, texture->id);
                diffuseNr++;
                totalNumber++;
            }
            else if (texture->type == "texture_specular")
            {
                if (shader->type == GLShader::LIGHTING)
                {
                    textureName = "material.specular[" + std::to_string(specularNr) + "]";
                    shader->setInt(textureName, totalNumber);
                    glActiveTexture(GL_TEXTURE0 + totalNumber);
                    glBindTexture(GL_TEXTURE_2D, texture->id);
                    specularNr++;
                    totalNumber++;
                }
            }
            else
            {
                std::cerr << "ERROR::INVALID_TEXTURE_TYPE:: " << texture->type << std::endl;
                throw new std::runtime_error("Check Logs");
            }
        }
    }

    glBindVertexArray(vao);
    if (shader->type == GLShader::NOLIGHTING) {shader->setInt("material.nDiffuseTexture", diffuseNr); }
    if (shader->type == GLShader::LIGHTING)
    {
        shader->setInt("material.nDiffuseTexture", diffuseNr);
        shader->setInt("material.nSpecularTexture", specularNr);
        shader->setFloat("material.shininess", 32.0f);
        shader->setVec3("viewPosition", Resources::currentApplication->renderer->currentCamera->orientation.cameraPosition);

        int numPointLights = 0, numSpotLights = 0, numDirectionalLights = 0;
        for (LightSource* lightSource : Resources::currentApplication->renderer->lightSourceRenderBuffer)
        {
            if (lightSource->lightType == LightSource::POINT)
            {
                shader->setVec3("pointLight[" + std::to_string(numPointLights) + "].ambient", lightSource->ambientColor);
                shader->setVec3("pointLight[" + std::to_string(numPointLights) + "].diffuse", lightSource->diffuseColor);
                shader->setVec3("pointLight[" + std::to_string(numPointLights) + "].specular", lightSource->specularColor);
                shader->setVec3("pointLight[" + std::to_string(numPointLights) + "].position", lightSource->position);
                shader->setVec3("pointLight[" + std::to_string(numPointLights) + "].attenuation", lightSource->attenuation);
                numPointLights++;
            }
            else if (lightSource->lightType == LightSource::DIRECTIONAL)
            {
                shader->setVec3("directionalLight[" + std::to_string(numDirectionalLights) + "].ambient", lightSource->ambientColor);
                shader->setVec3("directionalLight[" + std::to_string(numDirectionalLights) + "].diffuse", lightSource->diffuseColor);
                shader->setVec3("directionalLight[" + std::to_string(numDirectionalLights) + "].specular", lightSource->specularColor);
                shader->setVec3("directionalLight[" + std::to_string(numDirectionalLights) + "].direction", lightSource->lightDirection);
                numDirectionalLights++;
            }
            else if (lightSource->lightType == LightSource::SPOT)
            {
                if (lightSource->parentObject == LightSource::CAMERA)
                {
                    shader->setVec3("spotLight[" + std::to_string(numSpotLights) + "].position", Resources::currentApplication->renderer->currentCamera->orientation.cameraPosition);
                    shader->setVec3("spotLight[" + std::to_string(numSpotLights) + "].direction", Resources::currentApplication->renderer->currentCamera->orientation.cameraFront);
                }
                else if (lightSource->parentObject == LightSource::OBJECT)
                {
                    shader->setVec3("spotLight[" + std::to_string(numSpotLights) + "].position", lightSource->position);
                    shader->setVec3("spotLight[" + std::to_string(numSpotLights) + "].direction", lightSource->lightDirection);
                }
                else
                {
                    std::cerr << "ERROR::INVALID_LIGHT_PARENT_OBJECT" << std::endl;
                    throw std::runtime_error("Check Logs");
                }
                shader->setFloat("spotLight[" + std::to_string(numSpotLights) + "].innerCutOff", glm::cos(glm::radians(lightSource->innerCutOff)));
                shader->setFloat("spotLight[" + std::to_string(numSpotLights) + "].outerCutOff", glm::cos(glm::radians(lightSource->outerCutOff)));
                shader->setVec3("spotLight[" + std::to_string(numSpotLights) + "].ambient", lightSource->ambientColor);
                shader->setVec3("spotLight[" + std::to_string(numSpotLights) + "].diffuse", lightSource->diffuseColor);
                shader->setVec3("spotLight[" + std::to_string(numSpotLights) + "].specular", lightSource->specularColor);
                shader->setVec3("spotLight[" + std::to_string(numSpotLights) + "].attenuation", lightSource->attenuation);
                numSpotLights++;
            }
            else
            {
                std::cerr << "ERROR::INVALID_LIGHT_TYPE" << std::endl;
                throw std::runtime_error("Check Logs");
            }
        }
        shader->setInt("numPointLights", numPointLights);
        shader->setInt("numDirectionalLights", numDirectionalLights);
        shader->setInt("numSpotLights", numSpotLights);


    }
    if (shader->type == GLShader::LIGHTSOURCE)
    {
        shader->setVec3("lightColor", renderer->lightSource.sourceColor);
    }

        //Set up projection and view matrix
    Resources::currentApplication->renderer->currentCamera->processProjection(view, projection);
        //Set up model matrix
    scale = glm::scale(renderer->orientation->scale);
    rotation = glm::rotate(glm::radians(renderer->orientation->rotation.w), glm::vec3(renderer->orientation->rotation.x, renderer->orientation->rotation.y, renderer->orientation->rotation.z));
    translation = glm::translate(renderer->orientation->translation);
    model = translation * rotation * scale;
    // Send uniforms to object's shader
    shader->setMatrix4f("view", view);
    shader->setMatrix4f("projection", projection);
    shader->setMatrix4f("model", model);
    // Output current mesh
    if (output == DRAWELEMENTS) { glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0); }
    if (output == DRAWARRAYS) { glDrawArrays(GL_TRIANGLES, 0, vertices.size()); }



    glBindVertexArray(0);

    if (shader->type == GLShader::LIGHTSOURCE)
    {
        // Do Nothing
    }
    else {

        for (int i = 0; i < (diffuseNr + specularNr); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

    }
    
}