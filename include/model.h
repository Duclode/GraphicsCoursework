#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>

#include "Shader.h"
#include "mesh.h"

#include <string>
#include <iostream>
#include <vector>
#include <cfloat>

unsigned int TextureFromFile(const char* path,
                             const std::string &directory /* , bool gamma = false */);

class Model {
public:
    // constructor, expects a filepath to a 3D model.
    Model(const char* path) {
        loadModel(path);
    }

    void Draw(Shader &shader) {
        for(unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

    AABB GetWorldBounds(glm::mat4& modelMatrix) {
        AABB worldBounds;

        worldBounds.min = glm::vec3(FLT_MAX);
        worldBounds.max = glm::vec3(-FLT_MAX);

        for (const Mesh& mesh : meshes)
        {
            AABB meshBounds = mesh.GetWorldBounds(modelMatrix);

            worldBounds.min = glm::min(worldBounds.min, meshBounds.min);
            worldBounds.max = glm::max(worldBounds.max, meshBounds.max);
        }

        return worldBounds;
    }
private:
    // model data
    std::vector<Texture> textures_loaded; // stores already loaded textures
    std::vector<Mesh> meshes;
    std::string directory;

    void loadModel(std::string path) {
     // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(
            path,
            aiProcess_Triangulate |
            aiProcess_GenSmoothNormals |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace
        );
        // check for errors
        // if scene exists
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    // recursively processes each node and its children
    // scene contains all the data/obj, nodes containes indices to index scene for organisation
    void processNode(aiNode* node, const aiScene* scene) {
        // process all the node's meshes (if any)
        for(unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // then do the same for each of its children
        for(unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            // process vertex positions, normals and texture coordinates
            glm::vec3 vector;
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // normals
            if(mesh->HasNormals()) { // if mesh contains normal vectors
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            // textures
            if(mesh->mTextureCoords[0]) { // if mesh contains texture coordinates
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            } else {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }
            vertices.push_back(vertex);

        }
        // process indices
        for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for(unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        // process material
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        // diffuse maps
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material,
                                            aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // specular maps
        std::vector<Texture> specularMaps = loadMaterialTextures(material,
                                            aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // normal maps
        std::vector<Texture> normalMaps = loadMaterialTextures(material,
                                            aiTextureType_NORMALS, "texture_normal");
        // Fallback for exporters that store them as bump maps
        if (normalMaps.empty()) {
            normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        }

        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // height maps
        std::vector<Texture> heightMaps = loadMaterialTextures(material,
                                            aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        return Mesh(vertices, indices, textures);
    }

    // returns a Texture struct
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                              std::string typeName) {
        std::vector<Texture> textures{};
        for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            bool skip {false};
            for (unsigned int j {0}; j < textures_loaded.size(); j++) {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with same filepath has already been loaded
                    break;
                }
            }
            if (!skip) {
                // if texture hasn't been loaded already then load it
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture); // add to loaded textures
            }
        }
        return textures;
    }
};

inline unsigned int TextureFromFile(const char* path,
                                    const std::string &directory /* , bool gamma */ ) {
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID{};
    glGenTextures(1, &textureID);

    int width{};
    int height{};
    int nrComponents{};
    unsigned char *data {stbi_load(filename.c_str(), &width, &height, &nrComponents, 0)};
    if (data) {
        GLenum format{};
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        std::cout << "Failed: " << filename << '\n';
        std::cout << "Reason: " << stbi_failure_reason() << '\n';
        stbi_image_free(data);
    }
    return textureID;
}

#endif
