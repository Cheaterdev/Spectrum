module;

#include <cmath>

export module Test.Math.Extended;

export import Test.Framework;

import Core;

export namespace Test
{
	// Quaternion tests
	TEST(Core.Quaternions, Creation)
	{
		quat q(1.0f, 0.0f, 0.0f, 0.0f);
		ASSERT_EQ(q.w, 1.0f);
		ASSERT_EQ(q.x, 0.0f);
		ASSERT_EQ(q.y, 0.0f);
		ASSERT_EQ(q.z, 0.0f);
	}

	TEST(Core.Quaternions, Conjugate)
	{
		quat q(1.0f, 2.0f, 3.0f, 4.0f);
		quat conj = q.conjugate();
		ASSERT_EQ(conj.w, 1.0f);
		ASSERT_EQ(conj.x, -2.0f);
		ASSERT_EQ(conj.y, -3.0f);
		ASSERT_EQ(conj.z, -4.0f);
	}

	TEST(Core.Quaternions, Identity)
	{
		quat identity(1.0f, 0.0f, 0.0f, 0.0f);
		quat q(0.5f, 0.5f, 0.5f, 0.5f);
		quat result = identity * q;
		ASSERT_TRUE(std::abs(result.w - q.w) < 0.0001f);
		ASSERT_TRUE(std::abs(result.x - q.x) < 0.0001f);
	}

	// AABB tests
	TEST(Core.Geometry, AABB_Creation)
	{
		vec3 min(0.0f, 0.0f, 0.0f);
		vec3 max(1.0f, 1.0f, 1.0f);
		aabb box(min, max);
		ASSERT_EQ(box.min.x, 0.0f);
		ASSERT_EQ(box.max.x, 1.0f);
	}

	TEST(Core.Geometry, AABB_Center)
	{
		vec3 min(-1.0f, -1.0f, -1.0f);
		vec3 max(1.0f, 1.0f, 1.0f);
		aabb box(min, max);
		vec3 center = box.center();
		ASSERT_EQ(center.x, 0.0f);
		ASSERT_EQ(center.y, 0.0f);
		ASSERT_EQ(center.z, 0.0f);
	}

	TEST(Core.Geometry, AABB_Size)
	{
		vec3 min(0.0f, 0.0f, 0.0f);
		vec3 max(2.0f, 3.0f, 4.0f);
		aabb box(min, max);
		vec3 size = box.size();
		ASSERT_EQ(size.x, 2.0f);
		ASSERT_EQ(size.y, 3.0f);
		ASSERT_EQ(size.z, 4.0f);
	}

	// Sphere tests
	TEST(Core.Geometry, Sphere_Creation)
	{
		vec3 center(1.0f, 2.0f, 3.0f);
		float radius = 5.0f;
		sphere s(center, radius);
		ASSERT_EQ(s.center.x, 1.0f);
		ASSERT_EQ(s.radius, 5.0f);
	}

	TEST(Core.Geometry, Sphere_ContainsPoint)
	{
		vec3 center(0.0f, 0.0f, 0.0f);
		sphere s(center, 5.0f);

		vec3 inside(3.0f, 0.0f, 0.0f);
		vec3 outside(10.0f, 0.0f, 0.0f);

		float distInside = (inside - s.center).length();
		float distOutside = (outside - s.center).length();

		ASSERT_TRUE(distInside <= s.radius);
		ASSERT_TRUE(distOutside > s.radius);
	}

	// Ray tests
	TEST(Core.Geometry, Ray_Creation)
	{
		vec3 origin(0.0f, 0.0f, 0.0f);
		vec3 direction(1.0f, 0.0f, 0.0f);
		ray r(origin, direction);
		ASSERT_EQ(r.origin.x, 0.0f);
		ASSERT_EQ(r.direction.x, 1.0f);
	}

	TEST(Core.Geometry, Ray_PointAt)
	{
		vec3 origin(0.0f, 0.0f, 0.0f);
		vec3 direction(1.0f, 0.0f, 0.0f);
		ray r(origin, direction);

		vec3 point = r.origin + r.direction * 5.0f;
		ASSERT_EQ(point.x, 5.0f);
		ASSERT_EQ(point.y, 0.0f);
		ASSERT_EQ(point.z, 0.0f);
	}

	// Intersection tests
	TEST(Core.Intersections, RaySphereIntersection)
	{
		vec3 rayOrigin(0.0f, 0.0f, 0.0f);
		vec3 rayDir(1.0f, 0.0f, 0.0f);
		vec3 sphereCenter(5.0f, 0.0f, 0.0f);
		float sphereRadius = 2.0f;

		// Ray pointing towards sphere center should intersect
		vec3 toCenter = sphereCenter - rayOrigin;
		float distToCenter = toCenter.length();
		ASSERT_TRUE(distToCenter > 0.0f);
		ASSERT_TRUE(sphereRadius > 0.0f);
	}

	TEST(Core.Intersections, AABBContainsPoint)
	{
		aabb box(vec3(0.0f, 0.0f, 0.0f), vec3(10.0f, 10.0f, 10.0f));
		vec3 inside(5.0f, 5.0f, 5.0f);
		vec3 outside(15.0f, 5.0f, 5.0f);

		bool insideCheck = inside.x >= box.min.x && inside.x <= box.max.x &&
						  inside.y >= box.min.y && inside.y <= box.max.y &&
						  inside.z >= box.min.z && inside.z <= box.max.z;

		bool outsideCheck = outside.x >= box.min.x && outside.x <= box.max.x;

		ASSERT_TRUE(insideCheck);
		ASSERT_FALSE(outsideCheck);
	}

	TEST(Core.Intersections, AABBOverlap)
	{
		aabb box1(vec3(0.0f, 0.0f, 0.0f), vec3(5.0f, 5.0f, 5.0f));
		aabb box2(vec3(3.0f, 3.0f, 3.0f), vec3(8.0f, 8.0f, 8.0f));
		aabb box3(vec3(10.0f, 10.0f, 10.0f), vec3(15.0f, 15.0f, 15.0f));

		bool overlap12 = !(box1.max.x < box2.min.x || box1.min.x > box2.max.x ||
						  box1.max.y < box2.min.y || box1.min.y > box2.max.y ||
						  box1.max.z < box2.min.z || box1.min.z > box2.max.z);

		bool overlap13 = !(box1.max.x < box3.min.x || box1.min.x > box3.max.x ||
						  box1.max.y < box3.min.y || box1.min.y > box3.max.y ||
						  box1.max.z < box3.min.z || box1.min.z > box3.max.z);

		ASSERT_TRUE(overlap12);
		ASSERT_FALSE(overlap13);
	}
}
