#ifndef SPIRE_SHADER_BINDINGS
#define SPIRE_SHADER_BINDINGS

#define SPIRE_SHADER_BINDINGS_CONSTANT_SET 0
#define SPIRE_SHADER_BINDINGS_PER_FRAME_SET 1

#define SPIRE_SHADER_BINDINGS_VERTEX_SSBO_BINDING 0
#define SPIRE_SHADER_BINDINGS_MODEL_IMAGES_BINDING 1
#define SPIRE_SHADER_BINDINGS_CAMERA_UBO_BINDING 0
#define SPIRE_SHADER_BINDINGS_MODEL_DATA_SSBO_BINDING 1

#define SPIRE_SHADER_TEXTURE_COUNT 1

#define SPIRE_VOXEL_CHUNK_SIZE 64
#define SPIRE_VOXEL_CHUNK_AREA (SPIRE_VOXEL_CHUNK_SIZE * SPIRE_VOXEL_CHUNK_SIZE)
#define SPIRE_VOXEL_CHUNK_VOLUME (SPIRE_VOXEL_CHUNK_AREA * SPIRE_VOXEL_CHUNK_SIZE)

#define SPIRE_VOXEL_INDEX_TO_POSITION(positionType, index) \
    positionType( \
        (index) / SPIRE_VOXEL_CHUNK_AREA, \
        ((index) / SPIRE_VOXEL_CHUNK_SIZE) % SPIRE_VOXEL_CHUNK_SIZE, \
        (index) % SPIRE_VOXEL_CHUNK_SIZE \
        )

#define SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(x, y, z) \
    (x * SPIRE_VOXEL_CHUNK_AREA + y * SPIRE_VOXEL_CHUNK_SIZE + z)

#define SPIRE_VOXEL_POSITION_TO_INDEX(pos) SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(pos.x, pos.y, pos.z)


// vertex
#ifdef __cplusplus
namespace SpireVoxel {
#endif
    struct VertexData {
        int VoxelType;
        float x;
        float y;
        float z;
        float u;
        float v;
    };

#ifdef __cplusplus
}
#endif

#endif
