#pragma once

#include <vector>

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include "cgmath.h"
#include "cgut.h"

#include "_mesh.h"

class _model {
public:
    vec3 pos, size;

    _model();
    _model(vec3 pos = vec3(0.f), vec3 size = vec3(1.f))
        : pos(pos), size(size) {}

    void init();
    void load_model(std::string path);

    void render(_shader shader) {
        for (auto m : meshes) {
            m.render(shader);
        }
    }

    void clean_up() {
        for (auto m : meshes) {
            m.cleanup();
        }
    }

    void loadModel(std::string path) {
        Assimp::Importer importer;
        const auto scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "could not load model at " << path << std::endl;
            return;
        }
        processNode(scene->mRootNode, scene);
    }
    void processNode(aiNode* node, const aiScene* scene) {
        for (unsigned i = 0; i != node->mNumMeshes; ++i) {
            auto m = scene->mMeshes[node->mMeshes[i]];
            meshes.emplace_back(processMesh(mesh, scene));
        }

        for (unsigned i = 0; i != node->mNumChildren; ++i) {
            processNode(node->mChildren[i], scene);
        }
    }
    _mesh processMesh(aiMesh* mesh, const aiScene* scene) {
        std::vector<_vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<_texture> textures;
    }

protected:
    std::vector<_mesh> meshes;
    std::string directory;

    std::vector<_texture> textures_loaded;

    void process_node(aiNode* node, const aiScene* scene);
    _mesh process_mesh(aiMesh* mesh, const aiScene* scene);
    std::vector<_texture> load_textures(aiMaterial* mat, aiTextureType type);
};