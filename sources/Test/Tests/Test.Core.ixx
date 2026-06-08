module;

export module Test.Core;

export import Test.Framework;

import Core;

export namespace Test
{
	// Vector2 tests
	TEST(Core.Vectors, Vec2_Creation)
	{
		vec2 v(3.0f, 4.0f);
		ASSERT_EQ(v.x, 3.0f);
		ASSERT_EQ(v.y, 4.0f);
	}

	TEST(Core.Vectors, Vec2_Addition)
	{
		vec2 a(1.0f, 2.0f);
		vec2 b(3.0f, 4.0f);
		vec2 result = a + b;
		ASSERT_EQ(result.x, 4.0f);
		ASSERT_EQ(result.y, 6.0f);
	}

	TEST(Core.Vectors, Vec2_Subtraction)
	{
		vec2 a(5.0f, 7.0f);
		vec2 b(2.0f, 3.0f);
		vec2 result = a - b;
		ASSERT_EQ(result.x, 3.0f);
		ASSERT_EQ(result.y, 4.0f);
	}

	TEST(Core.Vectors, Vec2_DotProduct)
	{
		vec2 a(1.0f, 2.0f);
		vec2 b(3.0f, 4.0f);
		float result = vec2::dot(a, b);
		ASSERT_EQ(result, 11.0f);
	}

	TEST(Core.Vectors, Vec3_Creation)
	{
		vec3 v(1.0f, 2.0f, 3.0f);
		ASSERT_EQ(v.x, 1.0f);
		ASSERT_EQ(v.y, 2.0f);
		ASSERT_EQ(v.z, 3.0f);
	}

	TEST(Core.Vectors, Vec3_Addition)
	{
		vec3 a(1.0f, 2.0f, 3.0f);
		vec3 b(4.0f, 5.0f, 6.0f);
		vec3 result = a + b;
		ASSERT_EQ(result.x, 5.0f);
		ASSERT_EQ(result.y, 7.0f);
		ASSERT_EQ(result.z, 9.0f);
	}

	TEST(Core.Vectors, Vec3_CrossProduct)
	{
		vec3 a(1.0f, 0.0f, 0.0f);
		vec3 b(0.0f, 1.0f, 0.0f);
		vec3 result = vec3::cross(a, b);
		ASSERT_EQ(result.x, 0.0f);
		ASSERT_EQ(result.y, 0.0f);
		ASSERT_EQ(result.z, 1.0f);
	}

	TEST(Core.Vectors, Vec3_DotProduct)
	{
		vec3 a(1.0f, 2.0f, 3.0f);
		vec3 b(4.0f, 5.0f, 6.0f);
		float result = vec3::dot(a, b);
		ASSERT_EQ(result, 32.0f);
	}

	TEST(Core.Vectors, Vec3_Magnitude)
	{
		vec3 v(3.0f, 4.0f, 0.0f);
		float length = v.length();
		ASSERT_EQ(length, 5.0f);
	}

	TEST(Core.Vectors, Vec4_Creation)
	{
		vec4 v(1.0f, 2.0f, 3.0f, 4.0f);
		ASSERT_EQ(v.x, 1.0f);
		ASSERT_EQ(v.y, 2.0f);
		ASSERT_EQ(v.z, 3.0f);
		ASSERT_EQ(v.w, 4.0f);
	}

	TEST(Core.Vectors, Vec4_Addition)
	{
		vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
		vec4 b(5.0f, 6.0f, 7.0f, 8.0f);
		vec4 result = a + b;
		ASSERT_EQ(result.x, 6.0f);
		ASSERT_EQ(result.y, 8.0f);
		ASSERT_EQ(result.z, 10.0f);
		ASSERT_EQ(result.w, 12.0f);
	}

	TEST(Core.Matrices, Mat4x4_Identity)
	{
		mat4x4 m;
		m.identity();
		ASSERT_EQ(m.a11, 1.0f);
		ASSERT_EQ(m.a22, 1.0f);
		ASSERT_EQ(m.a33, 1.0f);
		ASSERT_EQ(m.a44, 1.0f);
		ASSERT_EQ(m.a12, 0.0f);
		ASSERT_EQ(m.a13, 0.0f);
	}

	TEST(Core.Matrices, Mat4x4_Translation)
	{
		vec3 trans(5.0f, 10.0f, 15.0f);
		mat4x4 m = mat4x4::translation(trans);
		ASSERT_EQ(m.a41, 5.0f);
		ASSERT_EQ(m.a42, 10.0f);
		ASSERT_EQ(m.a43, 15.0f);
	}

	TEST(Core.Vectors, Vec3_Normalize)
	{
		vec3 v(3.0f, 4.0f, 0.0f);
		vec3 normalized = v.normalize();
		float len = normalized.length();
		ASSERT_TRUE(len > 0.99f && len < 1.01f);
	}
}
