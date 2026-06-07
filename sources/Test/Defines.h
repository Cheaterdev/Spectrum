#pragma once
// =============================================================================
// Test/Defines.h  —  TEST LAYER
// Test framework and infrastructure for testing APIs.
// Chains upward to Spectrum/Defines.h.
// =============================================================================
#include "RenderSystem/Defines.h"

#define CONCAT(a, b) a##b
#define CONCAT_IMPL(a, b) CONCAT(a, b)

#define TEST(category, name) \
	void CONCAT_IMPL(test_, __LINE__)(); \
	Test::TestRegistrator CONCAT_IMPL(registrator_, __LINE__)(#category, #name, CONCAT_IMPL(test_, __LINE__), __FILE__, __LINE__); \
	void CONCAT_IMPL(test_, __LINE__)()

#define ASSERT_TRUE(condition) \
	Test::AssertTrue(condition, #condition, __FILE__, __LINE__)

#define ASSERT_FALSE(condition) \
	Test::AssertFalse(condition, #condition, __FILE__, __LINE__)

#define ASSERT_EQ(expected, actual) \
	Test::AssertEqual(expected, actual, __FILE__, __LINE__)

#define ASSERT_NE(expected, actual) \
	Test::AssertNotEqual(expected, actual, __FILE__, __LINE__)
