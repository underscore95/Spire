#include "EngineIncludes.h"
#include "../Assets/Shaders/ShaderInfo.h"
#include <gtest/gtest.h>

using namespace SpireVoxel;

using namespace SpireVoxel;

TEST(AmbientOcclusionTests, SetAndUnpack_SingleValue) {
    SPIRE_UINT32_TYPE packed = 0u;
    packed = SetAO(packed, 2, 0b01);

    EXPECT_EQ(UnpackAO(packed, 2), 0b01u);
}

TEST(AmbientOcclusionTests, SetAO_DifferentIndices_DoNotInterfere) {
    SPIRE_UINT32_TYPE packed = 0u;

    packed = SetAO(packed, 0, 0b01);
    packed = SetAO(packed, 5, 0b10);
    packed = SetAO(packed, 15, 0b11);

    EXPECT_EQ(UnpackAO(packed, 0),  0b01u);
    EXPECT_EQ(UnpackAO(packed, 5),  0b10u);
    EXPECT_EQ(UnpackAO(packed, 15), 0b11u);

    for (SPIRE_UINT32_TYPE i = 0; i < SPIRE_AO_VALUES_PER_U32; ++i) {
        if (i == 0 || i == 5 || i == 15) continue;
        EXPECT_EQ(UnpackAO(packed, i), 0u);
    }
}

TEST(AmbientOcclusionTests, SetAO_OverwriteSameIndex) {
    SPIRE_UINT32_TYPE packed = 0u;

    packed = SetAO(packed, 3, 0b01);
    packed = SetAO(packed, 3, 0b10);

    EXPECT_EQ(UnpackAO(packed, 3), 0b10u);
}

TEST(AmbientOcclusionTests, SetAO_AllIndices_MaxValues) {
    SPIRE_UINT32_TYPE packed = 0u;

    for (SPIRE_UINT32_TYPE i = 0; i < SPIRE_AO_VALUES_PER_U32; ++i) {
        packed = SetAO(packed, i, 0b11);
    }

    for (SPIRE_UINT32_TYPE i = 0; i < SPIRE_AO_VALUES_PER_U32; ++i) {
        EXPECT_EQ(UnpackAO(packed, i), 0b11u);
    }
}

TEST(AmbientOcclusionTests, RoundTrip_PatternedValues) {
    SPIRE_UINT32_TYPE packed = 0u;

    for (SPIRE_UINT32_TYPE i = 0; i < SPIRE_AO_VALUES_PER_U32; ++i) {
        packed = SetAO(packed, i, i % 4u);
    }

    for (SPIRE_UINT32_TYPE i = 0; i < SPIRE_AO_VALUES_PER_U32; ++i) {
        EXPECT_EQ(UnpackAO(packed, i), i % 4u);
    }
}
