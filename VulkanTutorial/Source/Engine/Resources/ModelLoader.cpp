#include "ModelLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>
#include "Mesh.h"

namespace Spire
{
    std::string GetPathRelativeToFile(const std::string& path, std::string_view pathToFile)
    {
        if (std::filesystem::path(path).is_absolute())
        {
            return path;
        }

        std::filesystem::path pathToFilePath(pathToFile);
        // make path absolute
        if (!pathToFilePath.is_absolute())
        {
            pathToFilePath = std::filesystem::absolute(pathToFilePath);
        }
        // remove file name from path
        pathToFilePath = pathToFilePath.remove_filename();
        // append path to file
        pathToFilePath /= path;
        return pathToFilePath.string();
    }

    template <typename T>
    static glm::u32 GetIndexOfElementAddIfMissing(std::vector<T>& vec, T& val)
    {
        for (glm::u32 i = 0; i < vec.size(); i++)
        {
            if (vec[i] == val) return i;
        }
        vec.push_back(val);
        return vec.size() - 1;
    }

    static std::unique_ptr<Mesh> ConvertMesh(const aiMesh* aiMesh, const aiScene* aiScene,
                                             std::vector<std::string>& texturePaths,
                                             std::string_view absoluteModelDirectoryPath)
    {
        DEBUG_ASSERT(aiMesh->mTextureCoords[0] != nullptr);
        // not starts or ends in \ or /
        if (absoluteModelDirectoryPath.size()) // might be empty if its directly in assets dir
        {
            DEBUG_ASSERT(absoluteModelDirectoryPath.back() != "\\"[0] || absoluteModelDirectoryPath.back() != "/"[0]);
            DEBUG_ASSERT(absoluteModelDirectoryPath.front() == "\\"[0] || absoluteModelDirectoryPath.front() == "/"[0]);
        }

        auto mesh = std::make_unique<Mesh>();

        // indices
        for (glm::u32 faceIndex = 0; faceIndex < aiMesh->mNumFaces; faceIndex++)
        {
            const aiFace& face = aiMesh->mFaces[faceIndex];
            DEBUG_ASSERT(face.mNumIndices == 3);

            // for (glm::u32 indexIndex = 0; indexIndex < face.mNumIndices; indexIndex++)
            // {
            //     const glm::u32 index = face.mIndices[indexIndex];
            //
            //     ModelVertex vert;
            //
            //     vert.Pos = {aiMesh->mVertices[index].x, aiMesh->mVertices[index].y, aiMesh->mVertices[index].z};
            //     vert.Tex = {aiMesh->mTextureCoords[0][index].x, aiMesh->mTextureCoords[0][index].y};
            //
            //     mesh->Vertices.push_back(vert);
            // }

            for (glm::u32 indexIndex = 0; indexIndex < face.mNumIndices; indexIndex++)
            {
                mesh->Indices.push_back(face.mIndices[indexIndex]);
            }
        }

        // vertices
        for (glm::u32 vertexIndex = 0; vertexIndex < aiMesh->mNumVertices; vertexIndex++)
        {
            ModelVertex vert = {};
            vert.Pos = {
                aiMesh->mVertices[vertexIndex].x, aiMesh->mVertices[vertexIndex].y, aiMesh->mVertices[vertexIndex].z
            };
            vert.Tex = {
                aiMesh->mTextureCoords[0][vertexIndex].x, aiMesh->mTextureCoords[0][vertexIndex].y
            };
            mesh->Vertices.push_back(vert);
        }

        // texture
        const auto material = aiScene->mMaterials[aiMesh->mMaterialIndex];
        aiString texturePath;
        material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
        DEBUG_ASSERT(!aiScene->GetEmbeddedTexture(texturePath.C_Str())); // don't support embedded textures
        std::string assetTexturePath = std::format("{}{}", absoluteModelDirectoryPath, texturePath.C_Str());
        mesh->TextureIndex = GetIndexOfElementAddIfMissing(texturePaths, assetTexturePath);

        return std::move(mesh);
    }

    static std::string GetFilePathRelativeToAssetsDirectoryNoFileName(std::string_view absoluteFileName)
    {
        std::string path(absoluteFileName);

        if (path.rfind(ASSETS_DIRECTORY, 0) == 0) // starts with
        {
            path.erase(0, strlen(ASSETS_DIRECTORY));
        }
        else
        {
            spdlog::warn("Path '{}' does not start with ASSETS_DIRECTORY '{}'", path, ASSETS_DIRECTORY);
        }

        std::filesystem::path fsPath(path);
        fsPath = fsPath.parent_path();

        std::string result = fsPath.generic_string();

        if (!result.empty() && result.front() == '/')
            result.erase(result.begin());

        if (!result.empty() && result.back() == '/')
            result.pop_back();

        return result;
    }

    Model ModelLoader::LoadModel(const char* fileName, std::vector<std::string>& texturePaths,
                                 const ModelLoadingSettings& settings)
    {
        ASSERT(settings.IgnoreNonTriangleMeshes); // Don't support not doing this rn
        // flags
        glm::u32 flags = 0;
        if (settings.TriangulateFaces) flags |= aiProcess_Triangulate;
        if (settings.FlipUVs) flags |= aiProcess_FlipUVs;
        flags |= aiProcess_SortByPType;

        // import
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(fileName, flags);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            spdlog::error("Failed to load model {} due to error '{}'", fileName, importer.GetErrorString());
            return {};
        }

        // convert assimp scene into a model
        Model model;
        model.reserve(scene->mNumMeshes);
        for (glm::u32 i = 0; i < scene->mNumMeshes; i++)
        {
            const auto material = scene->mMaterials[scene->mMeshes[i]->mMaterialIndex];

            // checks
            if (!scene->mMeshes[i]->HasTextureCoords(0))
            {
                spdlog::error(
                    "Failed to correctly import model {} (mesh {}) because it didn't contain texture coords in channel 0",
                    fileName, i);
                continue;
            }

            if (settings.IgnoreNonTriangleMeshes && (scene->mMeshes[i]->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) == 0)
            {
                spdlog::warn(
                    "Ignoring mesh {} of model {} because it contained non triangles (triangulation of polygons enabled: {})",
                    i, fileName, settings.TriangulateFaces);
                continue;
            }

            if (material->GetTextureCount(aiTextureType_DIFFUSE) != 1)
            {
                spdlog::error("Failed to correctly import model {} because mesh {} has {} diffuse textures", fileName, i,
                              material->GetTextureCount(aiTextureType_DIFFUSE));
                continue;
            }

            // convert
            model.push_back(ConvertMesh(scene->mMeshes[i], scene, texturePaths,
                                        GetFilePathRelativeToAssetsDirectoryNoFileName(fileName)));
        }

        importer.FreeScene();

        return model;
    }
}