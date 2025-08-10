#include "ModelLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>
#include "Mesh.h"

static std::unique_ptr<Mesh> ConvertMesh(const aiMesh* aiMesh)
{
    ASSERT(aiMesh->mTextureCoords[0] != nullptr);

    auto mesh = std::make_unique<Mesh>();
    for (glm::u32 faceIndex = 0; faceIndex < aiMesh->mNumFaces; faceIndex++)
    {
        const aiFace& face = aiMesh->mFaces[faceIndex];
        DEBUG_ASSERT(face.mNumIndices == 3);

        for (glm::u32 indexIndex = 0; indexIndex < face.mNumIndices; indexIndex++)
        {
            const glm::u32 index = face.mIndices[indexIndex];

            ModelVertex vert;

            vert.Pos = {aiMesh->mVertices[index].x, aiMesh->mVertices[index].y, aiMesh->mVertices[index].z};
            vert.Tex = {aiMesh->mTextureCoords[0][index].x, aiMesh->mTextureCoords[0][index].y};

            mesh->Vertices.push_back(vert);
        }
    }

    return std::move(mesh);
}

Model ModelLoader::LoadModel(const char* fileName, const ModelLoadingSettings& settings)
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
        if (!scene->mMeshes[i]->HasTextureCoords(0))
        {
            spdlog::error("Failed to import model {} because it didn't contain texture coords in channel 0", fileName);
            continue;
        }

        if (settings.IgnoreNonTriangleMeshes && (scene->mMeshes[i]->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) == 0)
        {
            spdlog::warn(
                "Ignoring mesh {} of model {} because it contained non triangles (triangulation of polygons enabled: {})",
                i, fileName, settings.TriangulateFaces);
            continue;
        }
        model.push_back(ConvertMesh(scene->mMeshes[i]));
    }

    return model;
}
