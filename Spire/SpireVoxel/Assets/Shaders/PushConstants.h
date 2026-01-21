#include "ShaderInfo.h"

#ifndef SPIRE_PUSH_CONSTANTS
#define SPIRE_PUSH_CONSTANTS

#ifdef __cplusplus
namespace SpireVoxel {
#endif

    struct
#ifdef __cplusplus
            alignas(16)
#endif
            PushConstantsData {
        // We use multiple vertex buffers, how many vertices does each buffer have?
        SPIRE_UINT32_TYPE NumVerticesPerBuffer;
    };

#ifdef __cplusplus
}
#endif

#ifndef __cplusplus
layout (push_constant) uniform PushConstants {
    PushConstantsData data;
} pushConstants;
#endif

#endif
