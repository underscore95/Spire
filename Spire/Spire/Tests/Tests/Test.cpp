#include <gtest/gtest.h>
#include "EngineIncludes.h"

// Demonstrate some basic assertions.
TEST(BasicAssertions, HelloTest) {
    // Expect two strings not to be equal.
    EXPECT_STRNE("hello", "world");
    // Expect equality.
    EXPECT_EQ(7 * 6, 42);
}
