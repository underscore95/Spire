#ifndef SPIRE_VOXEL_FACE_SIZE_H
#define SPIRE_VOXEL_FACE_SIZE_H
#include "ShaderInfo.h"

#ifdef __cplusplus
namespace SpireVoxel {
#endif

    // Face width and height can be calculated by comparing current vertex position with vertex position on the opposite side of the quad
    // These deltas map to the correct index
    // for example, for index 0 in a quad, index (0+2) has the opposite X value
    SPIRE_KEYWORD_INLINE const SPIRE_INT32_TYPE SPIRE_WIDTH_DELTAS[6] = {2, 1, -1, -2, -3, -1};

    // another example, for index 1 in a quad, index (1-1) has the opposite Y value
    SPIRE_KEYWORD_INLINE const SPIRE_INT32_TYPE SPIRE_HEIGHT_DELTAS[6] = {1, -1, -2, -3, -1, -2};

#ifdef __cplusplus
}
#endif

#endif
