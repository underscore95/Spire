// ReSharper disable CppVariableCanBeMadeConstexpr
#ifndef SPIRE_SHADER_BINDINGS
#define SPIRE_SHADER_BINDINGS

// Descriptor and descriptor set bindings
#define SPIRE_SHADER_BINDINGS_CONSTANT_SET 0
#define SPIRE_SHADER_BINDINGS_PER_FRAME_SET 1 // per frame, so takes 2 and 3 too
#define SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_SET 4

#define SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_BINDING 0
#define SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_VOXEL_DATA_BINDING 1
#define SPIRE_VOXEL_SHADER_BINDINGS_VOXEL_TYPE_UBO_BINDING 2
#define SPIRE_VOXEL_SHADER_BINDINGS_CHUNK_DATA_SSBO_BINDING 3
#define SPIRE_VOXEL_SHADER_BINDINGS_AO_DATA_BINDING 4

#define SPIRE_VOXEL_SHADER_BINDINGS_IMAGES_BINDING 1
#define SPIRE_SHADER_BINDINGS_CAMERA_UBO_BINDING 0

// Constants
#define SPIRE_SHADER_TEXTURE_COUNT 1

#define SPIRE_VOXEL_CHUNK_SIZE 64
#define SPIRE_VOXEL_CHUNK_AREA (SPIRE_VOXEL_CHUNK_SIZE * SPIRE_VOXEL_CHUNK_SIZE)
#define SPIRE_VOXEL_CHUNK_VOLUME (SPIRE_VOXEL_CHUNK_AREA * SPIRE_VOXEL_CHUNK_SIZE)

// Map from 3D index to 1D index
#define SPIRE_VOXEL_INDEX_TO_POSITION(positionType, index) \
    positionType( \
        (index) / SPIRE_VOXEL_CHUNK_AREA, \
        ((index) / SPIRE_VOXEL_CHUNK_SIZE) % SPIRE_VOXEL_CHUNK_SIZE, \
        (index) % SPIRE_VOXEL_CHUNK_SIZE \
        )

#define SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(x, y, z) \
    ((x) * SPIRE_VOXEL_CHUNK_AREA + (y) * SPIRE_VOXEL_CHUNK_SIZE + (z))

#define SPIRE_VOXEL_POSITION_TO_INDEX(pos) SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(pos.x, pos.y, pos.z)

// GPU doesn't have uint16, these macros unpack the uint16 from a uint32
#define SPIRE_VOXEL_UINT16_MAX 65535u

#define SPIRE_VOXEL_UNPACK_VOXEL_TYPE(doubleVoxelType, index) \
    (((index) % 2) == 0 ? (doubleVoxelType) & SPIRE_VOXEL_UINT16_MAX : (doubleVoxelType) >> 16)

#define SPIRE_VOXEL_PACK_VOXEL_TYPE(existingType, index, newType) \
    (((index) % 2) == 0 ? \
    (((existingType) & (SPIRE_VOXEL_UINT16_MAX << 16)) | ((newType) & SPIRE_VOXEL_UINT16_MAX)) : \
    (((existingType) & SPIRE_VOXEL_UINT16_MAX) | (((newType) & SPIRE_VOXEL_UINT16_MAX) << 16)))

// types
#ifdef __cplusplus
#define SPIRE_UINT32_TYPE glm::uint32
#define SPIRE_INT32_TYPE glm::int32
#define SPIRE_MAT4X4_TYPE glm::mat4
#define SPIRE_UVEC3_TYPE glm::uvec3
#define SPIRE_VEC3_TYPE glm::vec3
#define SPIRE_IVEC3_TYPE glm::ivec3
#define SPIRE_VEC2_TYPE glm::vec2
#define SPIRE_UVEC2_TYPE glm::uvec2
// reason for assert: ivec4 is put as padding on the gpu
// possible cause for failure: you didn't include vulkan before including this file
static_assert(sizeof(VkDrawIndirectCommand) == 16);
#define SPIRE_VK_INDIRECT_DRAW_COMMAND_TYPE(VarName) VkDrawIndirectCommand VarName
#else
#define SPIRE_UINT32_TYPE uint
#define SPIRE_INT32_TYPE int
#define SPIRE_MAT4X4_TYPE mat4
#define SPIRE_UVEC3_TYPE uvec3
#define SPIRE_VEC3_TYPE vec3
#define SPIRE_IVEC3_TYPE ivec3
#define SPIRE_VK_INDIRECT_DRAW_COMMAND_TYPE(VarName) \
    int VarName##_Padding1;\
    int VarName##_Padding2;\
    int VarName##_Padding3;\
    int VarName##_Padding4
#define SPIRE_VEC2_TYPE vec2
#define SPIRE_UVEC2_TYPE uvec2
#endif

// c++ keywords
#ifdef __cplusplus

#define SPIRE_KEYWORD_NODISCARD [[nodiscard]]
#define SPIRE_KEYWORD_STATIC static
#define SPIRE_KEYWORD_INLINE inline

#else

#define SPIRE_KEYWORD_NODISCARD
#define SPIRE_KEYWORD_STATIC
#define SPIRE_KEYWORD_INLINE

#endif

// chunk data
#ifdef __cplusplus
namespace SpireVoxel {
#endif
    // Chunk data, this exists on both CPU and GPU
    struct ChunkData {
        // Stores indirect draw command params, shader doesn't need to read this (but vulkan reads it)
        // this must be at the start of the struct
        SPIRE_VK_INDIRECT_DRAW_COMMAND_TYPE(CPU_DrawCommandParams);
        // Chunk coordinates
        SPIRE_INT32_TYPE ChunkX; // allows for 137 billion voxels in each direction
        SPIRE_INT32_TYPE ChunkY;
        SPIRE_INT32_TYPE ChunkZ;
        // Voxel data buffer contains all voxel data for the whole world, this index is where the latest data
        // for this chunk is
        SPIRE_UINT32_TYPE VoxelDataChunkIndex;
        SPIRE_UINT32_TYPE VoxelDataAllocationIndex;
        // What buffer is this chunks vertices stored in?
        SPIRE_UINT32_TYPE VertexBufferIndex;
        // LOD Scale, should be a uint but floating point operations are faster on gpu
        float LODScale;
        // AO data buffer contains all ambient occlusion data for the whole world, this index is where the latest data for this chunk is
        SPIRE_UINT32_TYPE AODataChunkPackedIndex; // This is uint index, not element index
        SPIRE_UINT32_TYPE AODataAllocationIndex;
    };

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace SpireVoxel {
#endif

    // faces
#define SPIRE_VOXEL_NUM_FACES 6
#define SPIRE_VOXEL_FACE_POS_X 0
#define SPIRE_VOXEL_FACE_NEG_X 1
#define SPIRE_VOXEL_FACE_POS_Y 2
#define SPIRE_VOXEL_FACE_NEG_Y 3
#define SPIRE_VOXEL_FACE_POS_Z 4
#define SPIRE_VOXEL_FACE_NEG_Z 5

#ifdef __cplusplus
    // Convert face enum to a string
    SPIRE_KEYWORD_NODISCARD SPIRE_KEYWORD_INLINE const char *FaceToString(SPIRE_UINT32_TYPE face) {
        static constexpr std::array<const char *, SPIRE_VOXEL_NUM_FACES> faces = {
            "PosX",
            "NegX",
            "PosY",
            "NegY",
            "PosZ",
            "NegZ",
        };
        if (face >= SPIRE_VOXEL_NUM_FACES) {
            return "Invalid Face";
        }
        return faces[face];
    }
#endif

    SPIRE_KEYWORD_NODISCARD SPIRE_KEYWORD_INLINE bool IsFaceOnXAxis(SPIRE_UINT32_TYPE face) {
        return face == SPIRE_VOXEL_FACE_POS_X || face == SPIRE_VOXEL_FACE_NEG_X;
    }

    SPIRE_KEYWORD_NODISCARD SPIRE_KEYWORD_INLINE bool IsFaceOnYAxis(SPIRE_UINT32_TYPE face) {
        return face == SPIRE_VOXEL_FACE_POS_Y || face == SPIRE_VOXEL_FACE_NEG_Y;
    }

    SPIRE_KEYWORD_NODISCARD SPIRE_KEYWORD_INLINE bool IsFaceOnZAxis(SPIRE_UINT32_TYPE face) {
        return face == SPIRE_VOXEL_FACE_POS_Z || face == SPIRE_VOXEL_FACE_NEG_Z;
    }

    // Convert from face enum to a normalised direction vector
    SPIRE_KEYWORD_NODISCARD SPIRE_KEYWORD_INLINE SPIRE_IVEC3_TYPE FaceToDirection(SPIRE_UINT32_TYPE face) {
        if (face == SPIRE_VOXEL_FACE_POS_X) return SPIRE_IVEC3_TYPE(1, 0, 0);
        if (face == SPIRE_VOXEL_FACE_NEG_X) return SPIRE_IVEC3_TYPE(-1, 0, 0);
        if (face == SPIRE_VOXEL_FACE_POS_Y) return SPIRE_IVEC3_TYPE(0, 1, 0);
        if (face == SPIRE_VOXEL_FACE_NEG_Y) return SPIRE_IVEC3_TYPE(0, -1, 0);
        if (face == SPIRE_VOXEL_FACE_POS_Z) return SPIRE_IVEC3_TYPE(0, 0, 1);
        if (face == SPIRE_VOXEL_FACE_NEG_Z) return SPIRE_IVEC3_TYPE(0, 0, -1);
#ifdef __cplusplus
        assert(false);
#endif
        return SPIRE_IVEC3_TYPE(0, 0, 0);
    }

    // Convert from a direction to a face enum
    SPIRE_KEYWORD_NODISCARD SPIRE_KEYWORD_INLINE SPIRE_UINT32_TYPE DirectionToFace(SPIRE_VEC3_TYPE direction) {
#ifdef __cplusplus
        assert(direction.x != 0 || direction.y != 0 || direction.z != 0);
#endif

        if (abs(direction.x) > abs(direction.y) && abs(direction.x) > abs(direction.z)) {
            return direction.x > 0 ? SPIRE_VOXEL_FACE_POS_X : SPIRE_VOXEL_FACE_NEG_X;
        }

        if (abs(direction.y) > abs(direction.x) && abs(direction.y) > abs(direction.z)) {
            return direction.y > 0 ? SPIRE_VOXEL_FACE_POS_Y : SPIRE_VOXEL_FACE_NEG_Y;
        }

        return direction.z > 0 ? SPIRE_VOXEL_FACE_POS_Z : SPIRE_VOXEL_FACE_NEG_Z;
    }

    // Voxel Type Face Layout
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

// greedy meshing face size swizzling
#ifdef __cplusplus
namespace SpireVoxel {
#endif
    SPIRE_KEYWORD_NODISCARD SPIRE_KEYWORD_INLINE SPIRE_UVEC3_TYPE GreedyMeshingFaceSizeSwizzleToWorldSpace(
        SPIRE_UINT32_TYPE face, SPIRE_UINT32_TYPE width, SPIRE_UINT32_TYPE height) {
        if (IsFaceOnZAxis(face)) return SPIRE_UVEC3_TYPE(width, height, 0);
        if (IsFaceOnYAxis(face)) return SPIRE_UVEC3_TYPE(width, 0, height);
#ifdef __cplusplus
        assert(IsFaceOnXAxis(face));
#endif
        return SPIRE_UVEC3_TYPE(0, height, width);
    }
#ifdef __cplusplus
}
#endif

// voxel type
#ifdef __cplusplus
namespace SpireVoxel {
    // Represents a registered voxel type on GPU, e.g. grass, dirt, etc
#endif
    struct GPUVoxelType {
        // Each type uses one or more textures, all textures are stored in a big array
        // This is the index into the array of the first texture
        SPIRE_UINT32_TYPE FirstTextureIndex;
        // See "Voxel Type Face Layout" comment, this decides how many textures are used by this voxel type
        // and what faces map to what textures (e.g. grass has a green top texture, grass/dirt transition texture on sides, and dirt texture on bottom
        SPIRE_UINT32_TYPE VoxelFaceLayout;
    };

#ifdef __cplusplus
}
#endif

// vertex
#ifdef __cplusplus
namespace SpireVoxel {
#endif
    // Utility functions to pack and unpack this type are below
    struct VertexData {
        SPIRE_UINT32_TYPE Packed_6Width6Height;
        SPIRE_UINT32_TYPE Packed_7X7Y7Z2VertPos3Face;
        SPIRE_UINT32_TYPE VoxelTypeStartingIndex;
    };

    // voxel vertex positions
#define SPIRE_NUM_VOXEL_VERTEX_POSITIONS 4
#ifdef __cplusplus
    enum class VoxelVertexPosition : SPIRE_UINT32_TYPE {
        ZERO, // uv 0, 0
        ONE, // uv 1, 0
        TWO, // uv 1, 1
        THREE // uv 0, 1
    };

#define SPIRE_VOXEL_VERTEX_POSITION_TYPE SpireVoxel::VoxelVertexPosition
#else
#define SPIRE_VOXEL_VERTEX_POSITION_TYPE SPIRE_UINT32_TYPE
#endif
    SPIRE_KEYWORD_NODISCARD SPIRE_KEYWORD_INLINE SPIRE_VEC2_TYPE VoxelVertexPositionToUV(SPIRE_VOXEL_VERTEX_POSITION_TYPE position) {
        SPIRE_KEYWORD_STATIC const SPIRE_VEC2_TYPE UV_COORDINATES[SPIRE_NUM_VOXEL_VERTEX_POSITIONS] =
#ifdef __cplusplus
        {
#else
               SPIRE_VEC2_TYPE[SPIRE_NUM_VOXEL_VERTEX_POSITIONS](
#endif
            SPIRE_VEC2_TYPE(0.0f, 0.0f),
            SPIRE_VEC2_TYPE(1.0f, 0.0f),
            SPIRE_VEC2_TYPE(1, 1.0f),
            SPIRE_VEC2_TYPE(0.0, 1.0f)
#ifdef __cplusplus
        };
        assert(static_cast<SPIRE_UINT32_TYPE>(position) < SPIRE_NUM_VOXEL_VERTEX_POSITIONS);
        return UV_COORDINATES[static_cast<SPIRE_UINT32_TYPE>(position)];
#else
);
        return UV_COORDINATES[position];
#endif
    }

    // packing and unpacking VertexData
#ifdef __cplusplus
    /*
     * VertexData Spec
     * uint32 Packed_8Width8Height; 32 bit uint where the first 6 bits are the width of the voxel face minus 1 and the next 6 bits are the height of the voxel face minus 1
     * uint32 Packed_7X7Y7Z2VertPos3Face; 32 bit uint where the first 7 bits the voxel Z coordinate in the chunk, second 7 bits is the Y coordinate, third 7 bits is the X coordinate,
     *      next 2 bits are VoxelVertexPosition, next 3 bits are voxel face (see SPIRE_VOXEL_NUM_FACES)
     */

    SPIRE_KEYWORD_NODISCARD SPIRE_KEYWORD_INLINE VertexData PackVertexData(
        SPIRE_UINT32_TYPE voxelTypeStartIndex,
        SPIRE_UINT32_TYPE width, // 1-64 range
        SPIRE_UINT32_TYPE height, // 1-64 range
        SPIRE_UINT32_TYPE x, SPIRE_UINT32_TYPE y, SPIRE_UINT32_TYPE z, // 0-64 range
        VoxelVertexPosition vertexPosition,
        glm::u16 face
    ) {
        assert(x <= 64);
        assert(y <= 64);
        assert(z <= 64);
        assert(width <= 64 && width > 0);
        assert(height <= 64 && height > 0);
        assert(face < SPIRE_VOXEL_NUM_FACES);
        width--;
        height--;
        const SPIRE_UINT32_TYPE MAX_SIX_BIT_VALUE = 63;
        VertexData vertex = {
            .Packed_6Width6Height = ((MAX_SIX_BIT_VALUE & height) << 6) | (MAX_SIX_BIT_VALUE & width),
            .Packed_7X7Y7Z2VertPos3Face = ((0b111 & face) << 23) | ((0b11 & static_cast<SPIRE_UINT32_TYPE>(vertexPosition)) << 21) | ((0xFF & x) << 14) | ((0xFF & y) << 7) | (
                                              0xFF & z),
            .VoxelTypeStartingIndex = voxelTypeStartIndex
        };

        return vertex;
    }
#endif

    SPIRE_KEYWORD_NODISCARD SPIRE_KEYWORD_INLINE SPIRE_UINT32_TYPE UnpackVertexDataFace(SPIRE_UINT32_TYPE packed) {
        const SPIRE_UINT32_TYPE MAX_THREE_BIT_VALUE = 7; // 0b111
        return (packed >> 23) & MAX_THREE_BIT_VALUE;
    }

    SPIRE_KEYWORD_NODISCARD SPIRE_KEYWORD_INLINE SPIRE_UVEC2_TYPE UnpackVertexFaceWidthHeight(SPIRE_UINT32_TYPE packed) {
        const SPIRE_UINT32_TYPE MAX_SIX_BIT_VALUE = 63;
        return SPIRE_UVEC2_TYPE(1 + (packed & MAX_SIX_BIT_VALUE), 1 + ((packed >> 6) & MAX_SIX_BIT_VALUE));
    }

    SPIRE_KEYWORD_NODISCARD SPIRE_KEYWORD_INLINE SPIRE_VOXEL_VERTEX_POSITION_TYPE UnpackVertexDataVertexPosition(SPIRE_UINT32_TYPE packed) {
        const SPIRE_UINT32_TYPE MAX_TWO_BIT_VALUE = 3; // 0b11
        SPIRE_UINT32_TYPE raw = (packed >> 21) & MAX_TWO_BIT_VALUE;
#ifdef __cplusplus
        assert(raw <= SPIRE_NUM_VOXEL_VERTEX_POSITIONS);
        return static_cast<SPIRE_VOXEL_VERTEX_POSITION_TYPE>(raw);
#else
        return raw;
#endif
    }

    SPIRE_KEYWORD_NODISCARD SPIRE_KEYWORD_INLINE SPIRE_UVEC3_TYPE UnpackVertexDataXYZ(SPIRE_UINT32_TYPE packed) {
        const SPIRE_UINT32_TYPE MAX_SEVEN_BIT_VALUE = 127; // 0b1111111
        return SPIRE_UVEC3_TYPE((packed >> 14) & MAX_SEVEN_BIT_VALUE, (packed >> 7) & MAX_SEVEN_BIT_VALUE, packed & MAX_SEVEN_BIT_VALUE);
    }

#ifdef __cplusplus
}
#endif

// camera
#ifdef __cplusplus
namespace SpireVoxel {
#endif
    // Stores camera matrices
    struct CameraInfo {
        SPIRE_MAT4X4_TYPE ViewProjectionMatrix;
        float Scale;
    };

#ifdef __cplusplus
}
#endif

// ambient occlusion
#ifdef __cplusplus
namespace SpireVoxel {
#endif
#define SPIRE_AO_VALUES_PER_U32 16

    SPIRE_KEYWORD_NODISCARD SPIRE_KEYWORD_INLINE SPIRE_UINT32_TYPE UnpackAO(SPIRE_UINT32_TYPE packed, SPIRE_UINT32_TYPE index) {
#ifdef __cplusplus
        assert(index < SPIRE_AO_VALUES_PER_U32);
#endif
        const SPIRE_UINT32_TYPE MAX_2_BIT_VALUE = 3; // 0b11
        return MAX_2_BIT_VALUE & (packed >> index * 2);
    }

    SPIRE_KEYWORD_NODISCARD SPIRE_KEYWORD_INLINE SPIRE_UINT32_TYPE SetAO(SPIRE_UINT32_TYPE packed, SPIRE_UINT32_TYPE index, SPIRE_UINT32_TYPE value) {
#ifdef __cplusplus
        assert(index < SPIRE_AO_VALUES_PER_U32);
        assert(value <= 0b11);
#endif
        const SPIRE_UINT32_TYPE MAX_2_BIT_VALUE = 3; // 0b11
        // clear the two bits we care about
        SPIRE_UINT32_TYPE clearMask = ~(MAX_2_BIT_VALUE << (index * 2));
        SPIRE_UINT32_TYPE clearedPacked = clearMask & packed;
        // set the two bits
        SPIRE_UINT32_TYPE valueToSet = value << (index * 2);
        return clearedPacked | valueToSet;
    }
#ifdef __cplusplus
}
#endif

#endif // SPIRE_SHADER_BINDINGS
