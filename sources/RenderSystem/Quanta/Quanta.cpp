module Graphics:Quanta;

import :Quanta;

import Core;

namespace Quanta
{
	namespace
	{
		uint32_t align_up(uint32_t v, uint32_t a) { return (v + a - 1) / a * a; }
	}

	float4 merge_spheres(const float4& a, const float4& b)
	{
		float3 ca(a.x, a.y, a.z), cb(b.x, b.y, b.z);
		float3 d = cb - ca;
		float dist = std::sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
		if (dist + b.w <= a.w) return a;
		if (dist + a.w <= b.w) return b;
		float r = (dist + a.w + b.w) * 0.5f;
		float3 c = ca + d * ((r - a.w) / dist);
		return float4(c.x, c.y, c.z, r);
	}

	Mesh Pack(std::vector<GroupInput>&& inputs, const Vertex* vertices, uint32_t original_triangles)
	{
		constexpr float Infinity = std::numeric_limits<float>::max();

		Mesh mesh;
		mesh.original_triangle_count = original_triangles;
		mesh.groups.reserve(inputs.size());

		std::vector<std::vector<uint8_t>> blobs(inputs.size());

		for (size_t g = 0; g < inputs.size(); g++)
		{
			const GroupInput& in = inputs[g];
			const uint32_t n = static_cast<uint32_t>(in.clusters.size());

			std::vector<ClusterHeader> headers(n);
			uint32_t cursor = align_up(sizeof(GroupHeader) + n * sizeof(ClusterHeader), BlobAlignment);
			for (uint32_t c = 0; c < n; c++)
			{
				const ClusterInput& cl = in.clusters[c];
				ClusterHeader& h = headers[c];
				h.vertex_count = static_cast<uint32_t>(cl.unique_vertices.size());
				h.triangle_count = static_cast<uint32_t>(cl.triangles.size() / 3);
				h.vertex_offset = cursor;
				cursor += h.vertex_count * sizeof(Vertex);
				h.triangle_offset = cursor;
				cursor = align_up(cursor + h.triangle_count * 3, BlobAlignment);
				h.bounding_sphere = cl.bounding_sphere;
				h.normal_cone = cl.normal_cone;
				h.lod_level = cl.lod_level;
				h.refined_error = cl.refined_error;
				h.reserved = 0;
				h.refined_sphere = cl.refined_sphere;

				ASSERT(h.vertex_count <= MaxClusterVertices && h.triangle_count <= MaxClusterTriangles);
				if (cl.refined_error < 0)
					mesh.original_cluster_count++;
			}
			ASSERT(cursor <= PageBytes);

			std::vector<uint8_t>& blob = blobs[g];
			blob.assign(cursor, 0);

			GroupHeader gh{};
			gh.cluster_count = n;
			gh.size_bytes = cursor;
			std::memcpy(blob.data(), &gh, sizeof(gh));
			std::memcpy(blob.data() + sizeof(GroupHeader), headers.data(), n * sizeof(ClusterHeader));

			for (uint32_t c = 0; c < n; c++)
			{
				const ClusterInput& cl = in.clusters[c];
				Vertex* dst = reinterpret_cast<Vertex*>(blob.data() + headers[c].vertex_offset);
				for (size_t v = 0; v < cl.unique_vertices.size(); v++)
					std::memcpy(dst + v, vertices + cl.unique_vertices[v], sizeof(Vertex));
				std::memcpy(blob.data() + headers[c].triangle_offset, cl.triangles.data(), cl.triangles.size());
			}

			Group rec{};
			rec.lod_sphere = in.lod_sphere;
			rec.refined_sphere = in.refined_sphere;
			rec.lod_error = in.lod_error;
			rec.refined_error = in.refined_error;
			rec.cluster_count = n;
			rec.depth = in.depth;
			rec.page = InvalidIndex;
			rec.size_bytes = cursor;
			mesh.groups.push_back(rec);
			mesh.max_depth = std::max(mesh.max_depth, in.depth);
		}

		for (uint32_t first = 0; first < mesh.groups.size(); first += NodeMaxGroups)
		{
			uint32_t count = std::min<uint32_t>(NodeMaxGroups, static_cast<uint32_t>(mesh.groups.size()) - first);

			Node node{};
			node.first_group = first;
			node.group_count = count;
			node.lod_sphere = mesh.groups[first].lod_sphere;
			node.lod_error = 0;
			node.refined_error = Infinity;
			for (uint32_t g = first; g < first + count; g++)
			{
				const Group& gr = mesh.groups[g];
				if (g > first)
					node.lod_sphere = merge_spheres(node.lod_sphere, gr.lod_sphere);
				node.lod_error = std::max(node.lod_error, gr.lod_error);
				node.refined_error = std::min(node.refined_error, gr.refined_error);
			}
			if (node.refined_error < 0)
			{
				node.refined_sphere = float4(0, 0, 0, 0);
			}
			else
			{
				node.refined_sphere = mesh.groups[first].refined_sphere;
				for (uint32_t g = first + 1; g < first + count; g++)
					node.refined_sphere = merge_spheres(node.refined_sphere, mesh.groups[g].refined_sphere);
			}
			mesh.nodes.push_back(node);
		}

		// Coarsest level first.
		std::vector<uint32_t> order;
		order.reserve(mesh.groups.size());
		for (uint32_t depth = mesh.max_depth + 1; depth-- > 0;)
			for (uint32_t g = 0; g < mesh.groups.size(); g++)
				if (mesh.groups[g].depth == depth)
					order.push_back(g);

		for (uint32_t g : order)
		{
			Group& rec = mesh.groups[g];
			if (mesh.pages.empty() || mesh.pages.back().data.size() + rec.size_bytes > PageBytes)
				mesh.pages.emplace_back();

			Page& page = mesh.pages.back();
			rec.page = static_cast<uint32_t>(mesh.pages.size() - 1);
			rec.offset_in_page = static_cast<uint32_t>(page.data.size());
			page.data.insert(page.data.end(), blobs[g].begin(), blobs[g].end());
			page.groups.push_back(g);
			if (rec.lod_error == Infinity)
				page.root = true;
		}

		for (const Page& page : mesh.pages)
			if (page.root)
				mesh.root_page_count++;

		return mesh;
	}

	GroupView read_group(const Mesh& mesh, uint32_t group)
	{
		const Group& rec = mesh.groups[group];
		GroupView view;
		view.blob = mesh.pages[rec.page].data.data() + rec.offset_in_page;
		view.header = reinterpret_cast<const GroupHeader*>(view.blob);
		view.clusters = reinterpret_cast<const ClusterHeader*>(view.blob + sizeof(GroupHeader));
		return view;
	}

	std::string validate(const Mesh& mesh)
	{
		constexpr float Infinity = std::numeric_limits<float>::max();

		if (mesh.groups.empty() || mesh.pages.empty())
			return "empty mesh";

		std::vector<uint32_t> placed(mesh.groups.size(), 0);
		for (uint32_t p = 0; p < mesh.pages.size(); p++)
		{
			const Page& page = mesh.pages[p];
			if (page.data.size() > PageBytes)
				return "page " + std::to_string(p) + " exceeds PageBytes";
			for (uint32_t g : page.groups)
			{
				if (g >= mesh.groups.size() || mesh.groups[g].page != p)
					return "page " + std::to_string(p) + " lists a group that points elsewhere";
				placed[g]++;
				if (mesh.groups[g].lod_error == Infinity && !page.root)
					return "page " + std::to_string(p) + " holds a root group but is not marked root";
			}
		}

		uint64_t original_triangles = 0;
		bool has_root = false;
		for (uint32_t g = 0; g < mesh.groups.size(); g++)
		{
			const Group& rec = mesh.groups[g];
			const std::string name = "group " + std::to_string(g);

			if (placed[g] != 1)
				return name + " is placed " + std::to_string(placed[g]) + " times";
			if (rec.offset_in_page % BlobAlignment != 0)
				return name + " is not aligned";
			if (rec.offset_in_page + rec.size_bytes > mesh.pages[rec.page].data.size())
				return name + " runs past its page";
			if (rec.cluster_count == 0 || rec.cluster_count > GroupMaxClusters)
				return name + " has a bad cluster count";
			has_root |= rec.lod_error == Infinity;

			GroupView view = read_group(mesh, g);
			if (view.header->cluster_count != rec.cluster_count || view.header->size_bytes != rec.size_bytes)
				return name + " header disagrees with its record";

			for (uint32_t c = 0; c < rec.cluster_count; c++)
			{
				const ClusterHeader& h = view.clusters[c];
				const std::string cname = name + " cluster " + std::to_string(c);

				if (h.vertex_count == 0 || h.vertex_count > MaxClusterVertices || h.triangle_count == 0 || h.triangle_count > MaxClusterTriangles)
					return cname + " exceeds the cluster limits";
				if (h.vertex_offset + h.vertex_count * sizeof(Vertex) > rec.size_bytes || h.triangle_offset + h.triangle_count * 3 > rec.size_bytes)
					return cname + " data runs past its blob";

				const Vertex* vertices = view.vertices(c);
				const uint8_t* triangles = view.triangles(c);
				for (uint32_t i = 0; i < h.triangle_count * 3; i++)
					if (triangles[i] >= h.vertex_count)
						return cname + " has a triangle index out of range";

				const float4& s = h.bounding_sphere;
				for (uint32_t v = 0; v < h.vertex_count; v++)
				{
					float3 d = vertices[v].position - float3(s.x, s.y, s.z);
					float dist = std::sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
					if (dist > s.w * 1.001f + 1e-4f)
						return cname + " has a vertex outside its bounding sphere";
				}

				if (h.refined_error < 0)
					original_triangles += h.triangle_count;
				else if (rec.lod_error != Infinity && h.refined_error > rec.lod_error)
					return cname + " has a refined error above its group's (LOD error not monotonic)";
			}
		}

		if (!has_root)
			return "no root group";
		if (original_triangles != mesh.original_triangle_count)
			return "original clusters hold " + std::to_string(original_triangles) + " triangles, expected " + std::to_string(mesh.original_triangle_count);

		uint32_t covered = 0;
		for (const Node& node : mesh.nodes)
		{
			if (node.first_group != covered || node.group_count == 0)
				return "nodes do not tile the groups";
			covered += node.group_count;
			for (uint32_t g = node.first_group; g < node.first_group + node.group_count; g++)
				if (mesh.groups[g].lod_error > node.lod_error)
					return "a node's error is below one of its groups";
		}
		if (covered != mesh.groups.size())
			return "nodes do not cover every group";

		return {};
	}
}
