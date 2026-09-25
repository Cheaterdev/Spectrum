module Graphics:DebugDraw;

import Core;
import HAL;

namespace
{
	vec3 transform_point(vec3 p, const mat4x4& m)
	{
		vec4 t = vec4(p, 1) * m;
		return vec3(t.xyz) / t.w;
	}

	// Any unit vector perpendicular to n.
	vec3 perpendicular(vec3 n)
	{
		vec3 axis = std::abs(n.x) < 0.9f ? vec3(1, 0, 0) : vec3(0, 1, 0);
		vec3 p = vec3::cross(n, axis);
		p.normalize();
		return p;
	}
}

void DebugDraw::line(vec3 a, vec3 b, float4 color, float thickness)
{
	Segment s;
	s.a = float4(a, thickness);
	s.b = float4(b, 0);
	s.color = color;

	std::lock_guard<std::mutex> g(m);
	pending.push_back(s);
}

void DebugDraw::box(vec3 min, vec3 max, float4 color, float thickness)
{
	mat4x4 identity;
	identity.identity();
	box(min, max, identity, color, thickness);
}

void DebugDraw::box(vec3 min, vec3 max, const mat4x4& transform, float4 color, float thickness)
{
	vec3 c[8];
	for (int i = 0; i < 8; i++)
	{
		vec3 p = { (i & 1) ? max.x : min.x, (i & 2) ? max.y : min.y, (i & 4) ? max.z : min.z };
		c[i] = transform_point(p, transform);
	}

	// Corner index bits are (x, y, z): an edge joins corners differing in one bit.
	for (int i = 0; i < 8; i++)
		for (int bit = 1; bit < 8; bit <<= 1)
			if (!(i & bit))
				line(c[i], c[i | bit], color, thickness);
}

void DebugDraw::frustum(const vec3 (&c)[8], float4 color, float thickness)
{
	static constexpr int edges[12][2] =
	{
		{ 0, 1 }, { 2, 3 }, { 0, 2 }, { 1, 3 },
		{ 4, 5 }, { 6, 7 }, { 4, 6 }, { 5, 7 },
		{ 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 },
	};

	for (auto& e : edges)
		line(c[e[0]], c[e[1]], color, thickness);
}

void DebugDraw::frustum(const mat4x4& inv_view_proj, float4 color, float thickness)
{
	vec3 c[8];
	for (int i = 0; i < 8; i++)
	{
		vec3 ndc = { (i & 1) ? 1.0f : -1.0f, (i & 2) ? 1.0f : -1.0f, (i & 4) ? 1.0f : 0.0f };
		c[i] = transform_point(ndc, inv_view_proj);
	}
	frustum(c, color, thickness);
}

void DebugDraw::circle(vec3 center, vec3 normal, float radius, float4 color, float thickness, int segments)
{
	normal.normalize();
	vec3 u = perpendicular(normal);
	vec3 v = vec3::cross(normal, u);

	vec3 prev = center + u * radius;
	for (int i = 1; i <= segments; i++)
	{
		float angle = Math::m_2_pi * float(i) / float(segments);
		vec3 p = center + (u * Math::cos(angle) + v * Math::sin(angle)) * radius;
		line(prev, p, color, thickness);
		prev = p;
	}
}

void DebugDraw::sphere(vec3 center, float radius, float4 color, float thickness, int segments)
{
	circle(center, vec3(1, 0, 0), radius, color, thickness, segments);
	circle(center, vec3(0, 1, 0), radius, color, thickness, segments);
	circle(center, vec3(0, 0, 1), radius, color, thickness, segments);
}

void DebugDraw::arrow(vec3 from, vec3 to, float4 color, float thickness)
{
	line(from, to, color, thickness);

	vec3 dir = to - from;
	float len = dir.length();
	if (len < 1e-5f)
		return;
	dir /= len;

	float head = len * 0.15f;
	vec3 u = perpendicular(dir);
	vec3 v = vec3::cross(dir, u);
	vec3 base = to - dir * head;
	for (vec3 side : { u, -u, v, -v })
		line(to, base + side * (head * 0.4f), color, thickness);
}

void DebugDraw::axes(const mat4x4& transform, float size, float thickness)
{
	vec3 origin = transform_point(vec3(0, 0, 0), transform);
	arrow(origin, transform_point(vec3(size, 0, 0), transform), float4(1, 0.2f, 0.2f, 1), thickness);
	arrow(origin, transform_point(vec3(0, size, 0), transform), float4(0.2f, 1, 0.2f, 1), thickness);
	arrow(origin, transform_point(vec3(0, 0, size), transform), float4(0.3f, 0.5f, 1, 1), thickness);
}

void DebugDraw::grid(vec3 center, float cell_size, int half_count, float4 color, float thickness)
{
	float extent = cell_size * half_count;
	for (int i = -half_count; i <= half_count; i++)
	{
		float o = cell_size * i;
		line(center + vec3(o, 0, -extent), center + vec3(o, 0, extent), color, thickness);
		line(center + vec3(-extent, 0, o), center + vec3(extent, 0, o), color, thickness);
	}
}

std::vector<DebugDraw::Segment> DebugDraw::take()
{
	std::lock_guard<std::mutex> g(m);
	std::vector<Segment> result;
	result.swap(pending);
	return result;
}
