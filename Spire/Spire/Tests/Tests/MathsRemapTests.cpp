#include <gtest/gtest.h>
#include "EngineIncludes.h"
#include "Utils/MathsUtils.h"

TEST(MathsRemapTests, Remap_IdentityRange) {
    EXPECT_FLOAT_EQ(Spire::MathsUtils::Remap(5.f, 0.f, 10.f, 0.f, 10.f), 5.f);
}

TEST(MathsRemapTests, Remap_LowerBound) {
    EXPECT_FLOAT_EQ(Spire::MathsUtils::Remap(0.f, 0.f, 10.f, 10.f, 20.f), 10.f);
}

TEST(MathsRemapTests, Remap_UpperBound) {
    EXPECT_FLOAT_EQ(Spire::MathsUtils::Remap(10.f, 0.f, 10.f, 10.f, 20.f), 20.f);
}

TEST(MathsRemapTests, Remap_MiddleValue) {
    EXPECT_FLOAT_EQ(Spire::MathsUtils::Remap(5.f, 0.f, 10.f, 10.f, 20.f), 15.f);
}

TEST(MathsRemapTests, Remap_ReversedOutputRange) {
    EXPECT_FLOAT_EQ(Spire::MathsUtils::Remap(0.f, 0.f, 10.f, 20.f, 10.f), 20.f);
    EXPECT_FLOAT_EQ(Spire::MathsUtils::Remap(10.f, 0.f, 10.f, 20.f, 10.f), 10.f);
    EXPECT_FLOAT_EQ(Spire::MathsUtils::Remap(5.f, 0.f, 10.f, 20.f, 10.f), 15.f);
}

TEST(MathsRemapTests, Remap_NegativeInputRange) {
    EXPECT_FLOAT_EQ(Spire::MathsUtils::Remap(-5.f, -10.f, 0.f, 0.f, 100.f), 50.f);
}

TEST(MathsRemapTests, Remap_NegativeOutputRange) {
    EXPECT_FLOAT_EQ(Spire::MathsUtils::Remap(5.f, 0.f, 10.f, -100.f, 0.f), -50.f);
}

TEST(MathsRemapTests, Remap_ValueOutsideRange) {
    EXPECT_FLOAT_EQ(Spire::MathsUtils::Remap(15.f, 0.f, 10.f, 0.f, 100.f), 150.f);
    EXPECT_FLOAT_EQ(Spire::MathsUtils::Remap(-5.f, 0.f, 10.f, 0.f, 100.f), -50.f);
}

TEST(MathsRemapTests, Remap_ReversedInputRange) {
    EXPECT_FLOAT_EQ(Spire::MathsUtils::Remap(5.f, 10.f, 0.f, 0.f, 100.f), 50.f);
}

TEST(MathsRemapTests, Remap_Vec2_MiddleValue) {
    glm::vec2 value(5.f, 2.5f);
    glm::vec2 currentMin(0.f, 0.f);
    glm::vec2 currentMax(10.f, 5.f);
    glm::vec2 newMin(10.f, 0.f);
    glm::vec2 newMax(20.f, 100.f);

    glm::ivec2 result = Spire::MathsUtils::Remap(value, currentMin, currentMax, newMin, newMax);

    EXPECT_EQ(result, glm::ivec2(15, 50));
}
