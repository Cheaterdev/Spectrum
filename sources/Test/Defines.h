#pragma once
// =============================================================================
// Test/Defines.h  —  TEST LAYER
// Test framework and infrastructure for testing APIs.
// Chains upward to Spectrum/Defines.h.
// =============================================================================
#include "RenderSystem/Defines.h"

#define TEST(category, name) \
	void Test_##category##_##name(); \
	Test::TestRegistrator registrator_##category##_##name(#category, #name, Test_##category##_##name, __FILE__, __LINE__); \
	void Test_##category##_##name()

#define ASSERT_TRUE(condition) \
	Test::AssertTrue(condition, #condition, __FILE__, __LINE__)

#define ASSERT_FALSE(condition) \
	Test::AssertFalse(condition, #condition, __FILE__, __LINE__)

#define ASSERT_EQ(expected, actual) \
	Test::AssertEqual(expected, actual, __FILE__, __LINE__)

#define ASSERT_NE(expected, actual) \
	Test::AssertNotEqual(expected, actual, __FILE__, __LINE__)
