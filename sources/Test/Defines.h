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

#define SETUP_CATEGORY(category, ...) \
	namespace { \
		struct CONCAT_IMPL(SetupRegistrar_, __LINE__) { \
			CONCAT_IMPL(SetupRegistrar_, __LINE__)() { \
				Test::TestRegistry::Instance().RegisterSetup(#category, __VA_ARGS__); \
			} \
		}; \
		static CONCAT_IMPL(SetupRegistrar_, __LINE__) CONCAT_IMPL(setup_reg_, __LINE__); \
	}

// Check a GPU texture against a saved PNG reference.
// Saves the texture as the reference if none exists.
// On mismatch: saves _actual.png and _diff.png under test_results/ and fails the test.
#define ASSERT_TEXTURE(tex, name) \
	Test::check_texture_reference(tex, name)

#define ASSERT_TEXTURE_EX(tex, name, sub_resource, tolerance) \
	Test::check_texture_reference(tex, name, sub_resource, tolerance)

#define TEARDOWN_CATEGORY(category, ...) \
	namespace { \
		struct CONCAT_IMPL(TeardownRegistrar_, __LINE__) { \
			CONCAT_IMPL(TeardownRegistrar_, __LINE__)() { \
				Test::TestRegistry::Instance().RegisterTeardown(#category, __VA_ARGS__); \
			} \
		}; \
		static CONCAT_IMPL(TeardownRegistrar_, __LINE__) CONCAT_IMPL(teardown_reg_, __LINE__); \
	}
