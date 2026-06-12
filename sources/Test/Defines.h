#pragma once
// =============================================================================
// Test/Defines.h  —  TEST LAYER
// Test framework and infrastructure for testing APIs.
// Chains upward to Spectrum/Defines.h.
// =============================================================================
#include "RenderSystem/Defines.h"

#define CONCAT(a, b) a##b
#define CONCAT_IMPL(a, b) CONCAT(a, b)

// Each test file must define TEST_MODULE_ID with a unique per-file token before
// using TEST().  The generated function names become test_{MODULE}_{LINE}, which
// are globally unique across all test modules.  Combined with `inline`, this
// prevents LNK2005 errors when MSVC copies function bodies across import boundaries.
//
//   Example (at file scope, before any TEST use):
//     #define TEST_MODULE_ID HALRendering

#define TEST(category, name) \
	inline void CONCAT_IMPL(CONCAT_IMPL(test_, TEST_MODULE_ID), CONCAT_IMPL(_, __LINE__))(); \
	inline ::Test::TestRegistrator CONCAT_IMPL(CONCAT_IMPL(registrator_, TEST_MODULE_ID), CONCAT_IMPL(_, __LINE__))(#category, #name, CONCAT_IMPL(CONCAT_IMPL(test_, TEST_MODULE_ID), CONCAT_IMPL(_, __LINE__)), __FILE__, __LINE__); \
	inline void CONCAT_IMPL(CONCAT_IMPL(test_, TEST_MODULE_ID), CONCAT_IMPL(_, __LINE__))()

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
