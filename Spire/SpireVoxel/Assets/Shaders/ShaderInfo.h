#ifndef SPIRE_SHADER_BINDINGS
#define SPIRE_SHADER_BINDINGS

#define SPIRE_SHADER_BINDINGS_CONSTANT_SET 0
#define SPIRE_SHADER_BINDINGS_PER_FRAME_SET 1 // per frame, so takes 2 and 3 too
#define SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_SET 4

#define SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_BINDING 0
#define SPIRE_VOXEL_SHADER_BINDINGS_VOXEL_TYPE_UBO_BINDING 2
#define SPIRE_VOXEL_SHADER_BINDINGS_CHUNK_DATA_SSBO_BINDING 3
#define SPIRE_VOXEL_SHADER_BINDINGS_IMAGES_BINDING 1
#define SPIRE_SHADER_BINDINGS_CAMERA_UBO_BINDING 0

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

// types
#ifdef __cplusplus
#define SPIRE_UINT32_TYPE glm::uint32
#define SPIRE_MAT4X4_TYPE glm::mat4
#else
#define SPIRE_UINT32_TYPE uint
#define SPIRE_MAT4X4_TYPE mat4
#endif

// c++ keywords
#ifdef __cplusplus

#define SPIRE_KEYWORD_INLINE inline
#define SPIRE_KEYWORD_NODISCARD [[nodiscard]]

#else

#define SPIRE_KEYWORD_NODISCARD
#define SPIRE_KEYWORD_INLINE

#endif

// chunk data
#ifdef __cplusplus
namespace SpireVoxel {
#endif
    struct ChunkData {
        SPIRE_UINT32_TYPE NumVertices;
        SPIRE_UINT32_TYPE FirstVertex;
    };

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace SpireVoxel {
#endif

    // faces
#define SPIRE_VOXEL_FACE_POS_X 0
#define SPIRE_VOXEL_FACE_NEG_X 1
#define SPIRE_VOXEL_FACE_POS_Y 2
#define SPIRE_VOXEL_FACE_NEG_Y 3
#define SPIRE_VOXEL_FACE_POS_Z 4
#define SPIRE_VOXEL_FACE_NEG_Z 5

    // face layout
#define SPIRE_VOXEL_LAYOUT_ALL_SAME 0 // all sides use image 0 (e.g. dirt)
#define SPIRE_VOXEL_LAYOUT_TOP_DIFFERENT_BOTTOM_DIFFERENT 1 // top uses image 0, bottom uses image 1, sides use image 2 (e.g. grass)

    // Get the number of images for a specific face layout
    SPIRE_KEYWORD_NODISCARD SPIRE_KEYWORD_INLINE SPIRE_UINT32_TYPE GetNumImages(SPIRE_UINT32_TYPE faceLayout) {
        if (faceLayout == SPIRE_VOXEL_LAYOUT_ALL_SAME) return 1;
        if (faceLayout == SPIRE_VOXEL_LAYOUT_TOP_DIFFERENT_BOTTOM_DIFFERENT) return 3;

#ifdef __cplusplus
        assert(false); // invalid face layout
#endif
        return 0;
    }

    // Get image index to use for a specific face layout and face
    SPIRE_KEYWORD_NODISCARD SPIRE_KEYWORD_INLINE SPIRE_UINT32_TYPE GetImageIndex(SPIRE_UINT32_TYPE faceLayout, SPIRE_UINT32_TYPE face) {
        if (faceLayout == SPIRE_VOXEL_LAYOUT_ALL_SAME) return 0;
        if (faceLayout == SPIRE_VOXEL_LAYOUT_TOP_DIFFERENT_BOTTOM_DIFFERENT) {
            if (face == SPIRE_VOXEL_FACE_POS_Y) return 0;
            if (face == SPIRE_VOXEL_FACE_NEG_Y) return 1;
            return 2;
        }

#ifdef __cplusplus
        assert(false); // invalid face layout
#endif
        return 0;
    }

#ifdef __cplusplus
}
#endif

// voxel type
#ifdef __cplusplus
namespace SpireVoxel {
#endif
    struct GPUVoxelType {
        SPIRE_UINT32_TYPE FirstTextureIndex;
        SPIRE_UINT32_TYPE VoxelFaceLayout;
    };

#ifdef __cplusplus
}
#endif

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
        SPIRE_UINT32_TYPE Face;
    };

#ifdef __cplusplus
}
#endif

// camera
#ifdef __cplusplus
namespace SpireVoxel {
#endif
    struct CameraInfo {
        SPIRE_MAT4X4_TYPE ViewProjectionMatrix;
        int TargetedVoxelX;
        int TargetedVoxelY;
        int TargetedVoxelZ;
        int IsTargetingVoxel;
    };

#ifdef __cplusplus
}
#endif

#endif // SPIRE_SHADER_BINDINGS
