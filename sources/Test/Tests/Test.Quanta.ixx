module;

export module Test.Quanta;

#define TEST_MODULE_ID Quanta

export import Test.Framework;

import Core;
import Graphics;

// Defined in RenderSystem/Quanta/QuantaBuilder.cpp (a plain TU).
extern "C++" Quanta::Mesh BuildQuanta(const uint32_t* indices, uint32_t index_count, const Quanta::Vertex* vertices, uint32_t vertex_count);

namespace
{
	struct TestMesh
	{
		std::vector<Quanta::Vertex> vertices;
		std::vector<uint32_t>       indices;
	};

	Quanta::Vertex make_vertex(float x, float y, float z, float nx, float ny, float nz, float u, float v)
	{
		Quanta::Vertex out{};
		out.position = float3(x, y, z);
		out.normal = float3(nx, ny, nz);
		out.uv = float2(u, v);
		out.tangent = float4(1, 0, 0, 1);
		return out;
	}

	// A closed mesh, the usual case: every edge has two triangles.
	TestMesh make_sphere(uint32_t slices, uint32_t stacks)
	{
		TestMesh m;
		for (uint32_t y = 0; y <= stacks; y++)
			for (uint32_t x = 0; x <= slices; x++)
			{
				float u = float(x) / slices, v = float(y) / stacks;
				float phi = v * 3.14159265f, theta = u * 2.0f * 3.14159265f;
				float nx = std::sin(phi) * std::cos(theta), ny = std::cos(phi), nz = std::sin(phi) * std::sin(theta);
				m.vertices.push_back(make_vertex(nx, ny, nz, nx, ny, nz, u, v));
			}
		for (uint32_t y = 0; y < stacks; y++)
			for (uint32_t x = 0; x < slices; x++)
			{
				uint32_t a = y * (slices + 1) + x, b = a + 1, c = a + slices + 1, d = c + 1;
				m.indices.insert(m.indices.end(), { a, c, b, b, c, d });
			}
		return m;
	}

	// An open mesh: the border has to stay put through simplification.
	TestMesh make_grid(uint32_t n)
	{
		TestMesh m;
		for (uint32_t y = 0; y <= n; y++)
			for (uint32_t x = 0; x <= n; x++)
				m.vertices.push_back(make_vertex(float(x), 0, float(y), 0, 1, 0, float(x) / n, float(y) / n));
		for (uint32_t y = 0; y < n; y++)
			for (uint32_t x = 0; x < n; x++)
			{
				uint32_t a = y * (n + 1) + x, b = a + 1, c = a + n + 1, d = c + 1;
				m.indices.insert(m.indices.end(), { a, c, b, b, c, d });
			}
		return m;
	}

	Quanta::Mesh build(const TestMesh& m)
	{
		return BuildQuanta(m.indices.data(), static_cast<uint32_t>(m.indices.size()), m.vertices.data(), static_cast<uint32_t>(m.vertices.size()));
	}
}

export namespace Test
{
	TEST(Quanta, SphereIsConsistent)
	{
		TestMesh sphere = make_sphere(256, 128);
		Quanta::Mesh mesh = build(sphere);
		ASSERT_EQ(std::string(), Quanta::validate(mesh));
		ASSERT_EQ(uint32_t(sphere.indices.size() / 3), mesh.original_triangle_count);
	}

	TEST(Quanta, OpenGridIsConsistent)
	{
		TestMesh grid = make_grid(200);
		Quanta::Mesh mesh = build(grid);
		ASSERT_EQ(std::string(), Quanta::validate(mesh));
	}

	TEST(Quanta, HasCoarserLevelsAndRoots)
	{
		Quanta::Mesh mesh = build(make_sphere(256, 128));
		ASSERT_TRUE(mesh.max_depth > 0);
		ASSERT_TRUE(mesh.root_page_count >= 1);
		ASSERT_TRUE(mesh.pages.front().root);
	}

	// The point of paging: what must stay resident is a small part of the whole.
	TEST(Quanta, RootPagesAreASmallPart)
	{
		Quanta::Mesh mesh = build(make_sphere(256, 128));
		ASSERT_TRUE(mesh.pages.size() > 1);

		size_t total = 0, roots = 0;
		for (auto& page : mesh.pages)
		{
			total += page.data.size();
			if (page.root)
				roots += page.data.size();
		}
		ASSERT_TRUE(roots * 4 < total);
	}

	TEST(Quanta, SameInputSameOutput)
	{
		TestMesh sphere = make_sphere(64, 32);
		Quanta::Mesh a = build(sphere), b = build(sphere);
		ASSERT_EQ(a.groups.size(), b.groups.size());
		ASSERT_EQ(a.pages.size(), b.pages.size());
		for (size_t p = 0; p < a.pages.size(); p++)
			ASSERT_TRUE(a.pages[p].data == b.pages[p].data);
	}
}
