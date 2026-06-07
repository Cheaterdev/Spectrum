#pragma once
// =============================================================================
// Test/Defines.h  —  TEST LAYER
// Test framework and infrastructure for testing APIs.
// Chains upward to Spectrum/Defines.h.
// =============================================================================
#include "RenderSystem/Defines.h"


#define TEST(name) \
	void Test_##name(); \
	Test::TestRegistrator registrator_##name(#name, Test_##name, __FILE__, __LINE__); \
	void Test_##name()

#define ASSERT_TRUE(condition) \
	Test::AssertTrue(condition, #condition, __FILE__, __LINE__)

#define ASSERT_FALSE(condition) \
	Test::AssertFalse(condition, #condition, __FILE__, __LINE__)

#define ASSERT_EQ(expected, actual) \
	Test::AssertEqual(expected, actual, __FILE__, __LINE__)

#define ASSERT_NE(expected, actual) \
	Test::AssertNotEqual(expected, actual, __FILE__, __LINE__)
