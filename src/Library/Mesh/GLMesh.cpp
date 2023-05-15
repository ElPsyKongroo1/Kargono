#include "Meshes.h"
#include "../Library.h"
/*============================================================================================================================================================================================
 *============================================================================================================================================================================================
 * GLMesh Class
 *============================================================================================================================================================================================
 *============================================================================================================================================================================================*/

 /*============================================================================================================================================================================================
  * GLMesh Functionality
  *============================================================================================================================================================================================*/

void GLMesh::setupMesh()
{
    output = GLMesh::DRAWARRAYS; // FIXME !!!!!!!

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


void GLMesh::Draw(void* object)
{
    Object* object0 = (Object*)object;
    // Matrix Local Variables
    glm::mat4 scale, rotation, translation;
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 projection, view;
    unsigned int diffuseNr = 0;
    unsigned int specularNr = 0;
    unsigned int totalNumber = 0;
    // Attach Appropriate texture units
    object0->shader->useProgram();

    std::string textureName;

    if (object0->shader->type == GLShader::NOTEXTURE)
    {
        // Do nothing
    }
    else
    {
        for (Texture texture : textures)
        {
            if (texture.type == "texture_diffuse")
            {
                textureName = "material.diffuse[" + std::to_string(diffuseNr) + "]";
                object0->shader->setInt(textureName, totalNumber);
                glActiveTexture(GL_TEXTURE0 + totalNumber);
                glBindTexture(GL_TEXTURE_2D, texture.id);
                diffuseNr++;
                totalNumber++;
            }
            else if (texture.type == "texture_specular")
            {
                if (object0->shader->type == GLShader::LIGHTING)
                {
                    textureName = "material.specular[" + std::to_string(specularNr) + "]";
                    object0->shader->setInt(textureName, totalNumber);
                    glActiveTexture(GL_TEXTURE0 + totalNumber);
                    glBindTexture(GL_TEXTURE_2D, texture.id);
                    specularNr++;
                    totalNumber++;
                }
            }
            else
            {
                std::cerr << "ERROR::INVALID_TEXTURE_TYPE:: " << texture.type << std::endl;
                throw new std::runtime_error("Check Logs");
            }
        }
    }

    glBindVertexArray(vao);
    if (object0->shader->type == GLShader::LIGHTING)
    {
        object0->shader->setInt("material.nDiffuseTexture", diffuseNr);
        object0->shader->setInt("material.nSpecularTexture", specularNr);
        object0->shader->setFloat("material.shininess", 32.0f);
        object0->shader->setVec3("viewPosition", Resources::currentApplication->currentCamera->cameraPosition);

        int numPointLights = 0, numSpotLights = 0, numDirectionalLights = 0;
        for (LightSource* lightSource : Resources::currentApplication->allLightSources)
        {
            if (lightSource->lightType == LightSource::POINT)
            {
                object0->shader->setVec3("pointLight[" + std::to_string(numPointLights) + "].ambient", lightSource->ambientColor);
                object0->shader->setVec3("pointLight[" + std::to_string(numPointLights) + "].diffuse", lightSource->diffuseColor);
                object0->shader->setVec3("pointLight[" + std::to_string(numPointLights) + "].specular", lightSource->specularColor);
                object0->shader->setVec3("pointLight[" + std::to_string(numPointLights) + "].position", lightSource->position);
                object0->shader->setVec3("pointLight[" + std::to_string(numPointLights) + "].attenuation", lightSource->attenuation);
                numPointLights++;
            }
            else if (lightSource->lightType == LightSource::DIRECTIONAL)
            {
                object0->shader->setVec3("directionalLight[" + std::to_string(numDirectionalLights) + "].ambient", lightSource->ambientColor);
                object0->shader->setVec3("directionalLight[" + std::to_string(numDirectionalLights) + "].diffuse", lightSource->diffuseColor);
                object0->shader->setVec3("directionalLight[" + std::to_string(numDirectionalLights) + "].specular", lightSource->specularColor);
                object0->shader->setVec3("directionalLight[" + std::to_string(numDirectionalLights) + "].direction", lightSource->lightDirection);
                numDirectionalLights++;
            }
            else if (lightSource->lightType == LightSource::SPOT)
            {
                if (lightSource->parentObject == LightSource::CAMERA)
                {
                    object0->shader->setVec3("spotLight[" + std::to_string(numSpotLights) + "].position", Resources::currentApplication->currentCamera->cameraPosition);
                    object0->shader->setVec3("spotLight[" + std::to_string(numSpotLights) + "].direction", Resources::currentApplication->currentCamera->cameraFront);
                }
                else if (lightSource->parentObject == LightSource::OBJECT)
                {
                    object0->shader->setVec3("spotLight[" + std::to_string(numSpotLights) + "].position", lightSource->position);
                    object0->shader->setVec3("spotLight[" + std::to_string(numSpotLights) + "].direction", lightSource->lightDirection);
                }
                else
                {
                    std::cerr << "ERROR::INVALID_LIGHT_PARENT_OBJECT" << std::endl;
                    throw std::runtime_error("Check Logs");
                }
                object0->shader->setFloat("spotLight[" + std::to_string(numSpotLights) + "].innerCutOff", glm::cos(glm::radians(lightSource->innerCutOff)));
                object0->shader->setFloat("spotLight[" + std::to_string(numSpotLights) + "].outerCutOff", glm::cos(glm::radians(lightSource->outerCutOff)));
                object0->shader->setVec3("spotLight[" + std::to_string(numSpotLights) + "].ambient", lightSource->ambientColor);
                object0->shader->setVec3("spotLight[" + std::to_string(numSpotLights) + "].diffuse", lightSource->diffuseColor);
                object0->shader->setVec3("spotLight[" + std::to_string(numSpotLights) + "].specular", lightSource->specularColor);
                object0->shader->setVec3("spotLight[" + std::to_string(numSpotLights) + "].attenuation", lightSource->attenuation);
                numSpotLights++;
            }
            else
            {
                std::cerr << "ERROR::INVALID_LIGHT_TYPE" << std::endl;
                throw std::runtime_error("Check Logs");
            }
        }
        object0->shader->setInt("numPointLights", numPointLights);
        object0->shader->setInt("numDirectionalLights", numDirectionalLights);
        object0->shader->setInt("numSpotLights", numSpotLights);


    }
    if (object0->shader->type == GLShader::NOTEXTURE)
    {
        object0->shader->setVec3("lightColor", object0->lightSource.sourceColor);
    }

        //Set up projection and view matrix
    Resources::currentApplication->currentCamera->processProjection(view, projection);
        //Set up model matrix
    scale = glm::scale(object0->scale);
    rotation = glm::rotate(glm::radians(object0->rotation.w), glm::vec3(object0->rotation.x, object0->rotation.y, object0->rotation.z));
    translation = glm::translate(object0->translation);
    model = translation * rotation * scale;
    // Send uniforms to object's shader
    object0->shader->setMatrix4f("view", view);
    object0->shader->setMatrix4f("projection", projection);
    object0->shader->setMatrix4f("model", model);
    // Output current mesh

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

    if (object0->shader->type == GLShader::NOTEXTURE)
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