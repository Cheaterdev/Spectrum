module;

#include <cmath>

export module Test.Math;

#define TEST_MODULE_ID Math

export import Test.Framework;

import stl.core;

export namespace Test
{
	// Basic arithmetic tests
	TEST(Arithmetic, Addition)
	{
		ASSERT_EQ(2 + 2, 4);
		ASSERT_EQ(10 + 5, 15);
		ASSERT_EQ(-1 + 1, 0);
	}

	TEST(Arithmetic, Subtraction)
	{
		ASSERT_EQ(5 - 3, 2);
		ASSERT_EQ(10 - 10, 0);
		ASSERT_EQ(0 - 5, -5);
	}

	TEST(Arithmetic, Multiplication)
	{
		ASSERT_EQ(3 * 4, 12);
		ASSERT_EQ(0 * 100, 0);
		ASSERT_EQ(-2 * 3, -6);
	}

	TEST(Arithmetic, Division)
	{
		ASSERT_EQ(10 / 2, 5);
		ASSERT_EQ(100 / 4, 25);
		ASSERT_EQ(7 / 2, 3);
	}

	TEST(Arithmetic, FloatingPoint)
	{
		float a = 1.5f + 2.5f;
		float expected = 4.0f;
		ASSERT_TRUE(std::abs(a - expected) < 0.0001f);
	}

	TEST(Arithmetic, Comparison)
	{
		ASSERT_TRUE(5 > 3);
		ASSERT_TRUE(3 < 5);
		ASSERT_EQ(5, 5);
		ASSERT_NE(5, 3);
	}
}
