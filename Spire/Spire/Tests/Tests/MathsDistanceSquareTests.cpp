#include <gtest/gtest.h>
#include "EngineIncludes.h"
#include "Utils/MathsUtils.h"

TEST(MathsDistanceSquareTests, Float) {
    for (float a = -5; a < 5; a += 0.5) {
        for (float b = -5; b < 5; b += 0.5) {
            EXPECT_FLOAT_EQ(Spire::MathsUtils::DistanceSquared(a, b), glm::distance(a, b) * glm::distance(a, b));
        }
    }
}

TEST(MathsDistanceSquareTests, Vec2) {
    for (float ax = -5; ax < 5; ax += 1.0f) {
        for (float ay = -5; ay < 5; ay += 1.0f) {
            for (float bx = -5; bx < 5; bx += 1.0f) {
                for (float by = -5; by < 5; by += 1.0f) {
                    glm::vec2 a(ax, ay);
                    glm::vec2 b(bx, by);
                    float expected = glm::distance(a, b);
                    expected *= expected;
                    EXPECT_FLOAT_EQ(Spire::MathsUtils::DistanceSquared(a, b), expected);
                }
            }
        }
    }
}

TEST(MathsDistanceSquareTests, Vec3) {
    for (float ax = -3; ax < 3; ax += 1.0f) {
        for (float ay = -3; ay < 3; ay += 1.0f) {
            for (float az = -3; az < 3; az += 1.0f) {
                for (float bx = -3; bx < 3; bx += 1.0f) {
                    for (float by = -3; by < 3; by += 1.0f) {
                        for (float bz = -3; bz < 3; bz += 1.0f) {
                            glm::vec3 a(ax, ay, az);
                            glm::vec3 b(bx, by, bz);
                            float expected = glm::distance(a, b);
                            expected *= expected;
                            EXPECT_FLOAT_EQ(Spire::MathsUtils::DistanceSquared(a, b), expected);
                        }
                    }
                }
            }
        }
    }
}

TEST(MathsDistanceSquareTests, Vec4) {
    for (float ax = -2; ax < 2; ax += 1.0f) {
        for (float ay = -2; ay < 2; ay += 1.0f) {
            for (float az = -2; az < 2; az += 1.0f) {
                for (float aw = -2; aw < 2; aw += 1.0f) {
                    for (float bx = -2; bx < 2; bx += 1.0f) {
                        for (float by = -2; by < 2; by += 1.0f) {
                            for (float bz = -2; bz < 2; bz += 1.0f) {
                                for (float bw = -2; bw < 2; bw += 1.0f) {
                                    glm::vec4 a(ax, ay, az, aw);
                                    glm::vec4 b(bx, by, bz, bw);
                                    float expected = glm::distance(a, b);
                                    expected *= expected;
                                    EXPECT_FLOAT_EQ(Spire::MathsUtils::DistanceSquared(a, b), expected);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
