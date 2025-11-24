#pragma once

#include <gtest/gtest.h>
#include "EngineIncludes.h"

#define EXPECT_IVEC3_EQ(actual, expected) \
do { \
glm::ivec3 a = actual; \
glm::ivec3 e = expected; \
EXPECT_TRUE(a == e) << "Expected: (" << e.x << "," << e.y << "," << e.z << "), " \
<< "Actual: (" << a.x << "," << a.y << "," << a.z << ")"; \
} while(0)

#define EXPECT_UVEC3_EQ(actual, expected) \
do { \
glm::uvec3 a = actual; \
glm::uvec3 e = expected; \
EXPECT_TRUE(a == e) << "Expected: (" << e.x << "," << e.y << "," << e.z << "), " \
<< "Actual: (" << a.x << "," << a.y << "," << a.z << ")"; \
} while(0)
