module Graphics:Quanta;

import :Quanta;

import Core;
import HAL;

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
				h.refined_family = cl.refined_family;
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
			rec.family = in.family;
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
			page.size = static_cast<uint32_t>(page.data.size());
			page.groups.push_back(g);
			if (rec.lod_error == Infinity)
				page.root = true;
		}

		for (const Page& page : mesh.pages)
			if (page.root)
				mesh.root_page_count++;

		build_page_parents(mesh);
		return mesh;
	}

	// A page can be shown only once the pages holding the groups that refine into it (its coarser parents) are:
	// otherwise the cut would find the fine groups and no stand-in for them.
	void build_page_parents(Mesh& mesh)
	{
		for (Page& page : mesh.pages)
			page.parents.clear();

		for (uint32_t q = 0; q < mesh.groups.size(); q++)
		{
			const Group& coarse = mesh.groups[q];
			GroupView view = read_group(mesh, q);
			for (uint32_t c = 0; c < coarse.cluster_count; c++)
			{
				const ClusterHeader& header = view.clusters[c];
				if (header.refined_error < 0)
					continue;

				const uint32_t first = family_first(header.refined_family);
				for (uint32_t i = 0; i < family_count(header.refined_family); i++)
				{
					const uint32_t fine_page = mesh.groups[first + i].page;
					if (fine_page == coarse.page)
						continue;
					auto& parents = mesh.pages[fine_page].parents;
					if (std::find(parents.begin(), parents.end(), coarse.page) == parents.end())
						parents.push_back(coarse.page);
				}
			}
		}
		mesh.parents_built = true;
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

	namespace
	{
		constexpr uint32_t FileMagic = 0x41544E51;   // "QNTA"
		constexpr uint32_t FileVersion = 4;   // 2: groups and cluster headers carry their family links; 3: pages may be GDeflate; 4: the page parent table
		constexpr uint32_t FileVersionNoParents = 3;

		bool keep_pages_in_memory = false;

		struct FileHeader
		{
			uint32_t magic;
			uint32_t version;
			uint32_t page_bytes;
			uint32_t group_count;
			uint32_t node_count;
			uint32_t page_count;
			uint32_t page_group_ids;       // length of the flattened page -> group list
			uint32_t original_cluster_count;
			uint32_t original_triangle_count;
			uint32_t max_depth;
			uint32_t root_page_count;
			uint32_t parent_dwords;        // length of the page parent table: per page, a count then that many page numbers
		};
		static_assert(sizeof(FileHeader) == FileHeaderBytes);

		struct PageEntry
		{
			uint64_t offset;
			uint32_t size;
			uint32_t root;
			uint32_t group_count;
			uint32_t first_group_id;       // into the flattened list
			uint32_t stored_size;          // bytes in the file: `size` for a raw page, fewer for a GDeflate one
			uint32_t reserved;
		};
		static_assert(std::is_trivially_copyable_v<FileHeader> && std::is_trivially_copyable_v<PageEntry>);

		template<class T>
		void append(std::vector<uint8_t>& out, const T* items, size_t count)
		{
			const uint8_t* bytes = reinterpret_cast<const uint8_t*>(items);
			out.insert(out.end(), bytes, bytes + count * sizeof(T));
		}
	}

	std::vector<uint8_t> serialize(const Mesh& mesh)
	{
		BlobPlan plan = plan_blob(mesh);
		std::vector<uint8_t> out = std::move(plan.metadata);
		out.reserve(plan.size);
		plan.metadata.resize(out.size());   // write_blob_pages only needs to know where the metadata ends
		write_blob_pages(mesh, plan, [&](const uint8_t* bytes, size_t count) { out.insert(out.end(), bytes, bytes + count); });
		return out;
	}

	BlobPlan plan_blob(const Mesh& mesh)
	{
		BlobPlan plan;
		plan.pages.resize(mesh.pages.size());

		FileHeader header{};
		header.magic = FileMagic;
		header.version = FileVersion;
		header.page_bytes = PageBytes;
		header.group_count = static_cast<uint32_t>(mesh.groups.size());
		header.node_count = static_cast<uint32_t>(mesh.nodes.size());
		header.page_count = static_cast<uint32_t>(mesh.pages.size());
		header.original_cluster_count = mesh.original_cluster_count;
		header.original_triangle_count = mesh.original_triangle_count;
		header.max_depth = mesh.max_depth;
		header.root_page_count = mesh.root_page_count;

		std::vector<uint32_t> group_ids;
		std::vector<PageEntry> entries(mesh.pages.size());
		for (size_t p = 0; p < mesh.pages.size(); p++)
		{
			const Page& page = mesh.pages[p];
			BlobPlan::PagePart& part = plan.pages[p];
			if (!page.data.empty())
			{
				auto packed = HAL::gdeflate_compress(page.data.data(), page.data.size());
				part.bytes = (!packed.empty() && packed.size() < page.data.size()) ? std::move(packed) : page.data;
				entries[p].size = static_cast<uint32_t>(page.data.size());
				part.stored_size = static_cast<uint32_t>(part.bytes.size());
			}
			else if (page.size != 0)
			{
				// Still in the file it was loaded from: copied as stored.
				if (mesh.file.empty() || page.file_size == 0)
					throw std::runtime_error("Quanta page has neither its bytes nor a place in an asset file");
				entries[p].size = page.size;
				part.stored_size = page.file_size;
				part.file_offset = page.file_offset;
			}

			entries[p].stored_size = part.stored_size;
			entries[p].root = mesh.pages[p].root ? 1 : 0;
			entries[p].group_count = static_cast<uint32_t>(mesh.pages[p].groups.size());
			entries[p].first_group_id = static_cast<uint32_t>(group_ids.size());
			group_ids.insert(group_ids.end(), mesh.pages[p].groups.begin(), mesh.pages[p].groups.end());
		}
		header.page_group_ids = static_cast<uint32_t>(group_ids.size());

		std::vector<uint32_t> parent_table;
		for (const Page& page : mesh.pages)
		{
			parent_table.push_back(static_cast<uint32_t>(page.parents.size()));
			parent_table.insert(parent_table.end(), page.parents.begin(), page.parents.end());
		}
		header.parent_dwords = static_cast<uint32_t>(parent_table.size());

		uint64_t cursor = sizeof(FileHeader) + mesh.groups.size() * sizeof(Group) + mesh.nodes.size() * sizeof(Node)
			+ entries.size() * sizeof(PageEntry) + group_ids.size() * sizeof(uint32_t) + parent_table.size() * sizeof(uint32_t);
		for (size_t p = 0; p < entries.size(); p++)
		{
			cursor = align_up(static_cast<uint32_t>(cursor), BlobAlignment);
			entries[p].offset = cursor;
			plan.pages[p].blob_offset = cursor;
			cursor += entries[p].stored_size;
		}
		plan.size = cursor;

		std::vector<uint8_t>& out = plan.metadata;
		append(out, &header, 1);
		append(out, mesh.groups.data(), mesh.groups.size());
		append(out, mesh.nodes.data(), mesh.nodes.size());
		append(out, entries.data(), entries.size());
		append(out, group_ids.data(), group_ids.size());
		append(out, parent_table.data(), parent_table.size());
		return plan;
	}

	void write_blob_pages(const Mesh& mesh, const BlobPlan& plan, const std::function<void(const uint8_t*, size_t)>& write)
	{
		static const uint8_t padding[BlobAlignment] = {};

		std::ifstream file;
		std::vector<uint8_t> stored;
		uint64_t cursor = plan.metadata.size();

		auto pad_to = [&](uint64_t offset)
		{
			while (cursor < offset)
			{
				const size_t count = static_cast<size_t>(std::min<uint64_t>(offset - cursor, sizeof(padding)));
				write(padding, count);
				cursor += count;
			}
		};

		for (const BlobPlan::PagePart& part : plan.pages)
		{
			if (part.stored_size == 0)
				continue;

			pad_to(part.blob_offset);
			if (!part.bytes.empty())
				write(part.bytes.data(), part.bytes.size());
			else
			{
				if (!file.is_open())
				{
					file.open(mesh.file, std::ios::binary);
					if (!file)
						throw std::runtime_error("Quanta page cannot be read back from its asset file");
				}
				stored.resize(part.stored_size);
				file.seekg(static_cast<std::streamoff>(part.file_offset));
				file.read(reinterpret_cast<char*>(stored.data()), static_cast<std::streamsize>(stored.size()));
				if (!file)
					throw std::runtime_error("Quanta page cannot be read back from its asset file");
				write(stored.data(), stored.size());
			}
			cursor += part.stored_size;
		}
		pad_to(plan.size);
	}

	bool metadata_size(const uint8_t* header_bytes, uint64_t& bytes, bool& needs_pages)
	{
		FileHeader header;
		std::memcpy(&header, header_bytes, sizeof(header));
		if (header.magic != FileMagic || (header.version != FileVersion && header.version != FileVersionNoParents) || header.page_bytes != PageBytes)
			return false;

		needs_pages = header.version < FileVersion;
		bytes = sizeof(FileHeader) + uint64_t(header.group_count) * sizeof(Group) + uint64_t(header.node_count) * sizeof(Node)
			+ uint64_t(header.page_count) * sizeof(PageEntry) + uint64_t(header.page_group_ids) * sizeof(uint32_t)
			+ (header.version >= FileVersion ? uint64_t(header.parent_dwords) * sizeof(uint32_t) : 0);
		return true;
	}

	bool keep_page_data() { return keep_pages_in_memory; }
	void set_keep_page_data(bool keep) { keep_pages_in_memory = keep; }

	bool deserialize(const uint8_t* data, size_t size, Mesh& out, uint64_t base_file_offset, bool keep_pages, uint64_t blob_size)
	{
		if (size < sizeof(FileHeader))
			return false;

		FileHeader header;
		std::memcpy(&header, data, sizeof(header));
		if (header.magic != FileMagic || (header.version != FileVersion && header.version != FileVersionNoParents) || header.page_bytes != PageBytes)
			return false;

		const bool has_parents = header.version >= FileVersion;
		const bool read_bytes = keep_pages || !has_parents;
		const uint64_t total = (blob_size && !read_bytes) ? blob_size : size;

		uint64_t cursor = sizeof(FileHeader);
		const uint64_t groups_at = cursor;
		cursor += uint64_t(header.group_count) * sizeof(Group);
		const uint64_t nodes_at = cursor;
		cursor += uint64_t(header.node_count) * sizeof(Node);
		const uint64_t entries_at = cursor;
		cursor += uint64_t(header.page_count) * sizeof(PageEntry);
		const uint64_t ids_at = cursor;
		cursor += uint64_t(header.page_group_ids) * sizeof(uint32_t);
		const uint64_t parents_at = cursor;
		if (has_parents)
			cursor += uint64_t(header.parent_dwords) * sizeof(uint32_t);
		if (cursor > size)
			return false;

		Mesh mesh;
		{
			mesh.groups.resize(header.group_count);
			mesh.nodes.resize(header.node_count);
			if (header.group_count)
				std::memcpy(mesh.groups.data(), data + groups_at, header.group_count * sizeof(Group));
			if (header.node_count)
				std::memcpy(mesh.nodes.data(), data + nodes_at, header.node_count * sizeof(Node));
		}
		mesh.original_cluster_count = header.original_cluster_count;
		mesh.original_triangle_count = header.original_triangle_count;
		mesh.max_depth = header.max_depth;
		mesh.root_page_count = header.root_page_count;

		{
			std::vector<PageEntry> entries(header.page_count);
			if (header.page_count)
				std::memcpy(entries.data(), data + entries_at, header.page_count * sizeof(PageEntry));
			std::vector<uint32_t> group_ids(header.page_group_ids);
			if (header.page_group_ids)
				std::memcpy(group_ids.data(), data + ids_at, header.page_group_ids * sizeof(uint32_t));

			mesh.pages.resize(header.page_count);
			for (uint32_t p = 0; p < header.page_count; p++)
			{
				const PageEntry& e = entries[p];
				if (e.size > PageBytes || e.stored_size > e.size || e.offset + e.stored_size > total)
					return false;
				if (uint64_t(e.first_group_id) + e.group_count > group_ids.size())
					return false;

				Page& page = mesh.pages[p];
				page.size = e.size;
				if (!read_bytes)
				{
					// Left in the file: read_page() fetches it when it is loaded.
				}
				else if (e.stored_size == e.size)
					page.data.assign(data + e.offset, data + e.offset + e.size);
				else
				{
					// The memory copy stays raw: only the file (and DirectStorage's read of it) is compressed.
					page.data.resize(e.size);
					if (!HAL::gdeflate_decompress(data + e.offset, e.stored_size, page.data.data(), e.size))
						return false;
				}
				page.root = e.root != 0;
				page.file_offset = base_file_offset + e.offset;
				page.file_size = e.stored_size;
				page.groups.reserve(e.group_count);
				for (uint32_t i = 0; i < e.group_count; i++)
				{
					uint32_t g = group_ids[e.first_group_id + i];
					if (g >= header.group_count)
						return false;
					page.groups.push_back(g);
				}
			}
		}

		if (has_parents)
		{
			std::vector<uint32_t> table(header.parent_dwords);
			if (header.parent_dwords)
				std::memcpy(table.data(), data + parents_at, header.parent_dwords * sizeof(uint32_t));

			size_t at = 0;
			for (uint32_t p = 0; p < header.page_count; p++)
			{
				if (at >= table.size() || at + 1 + table[at] > table.size())
					return false;
				const uint32_t count = table[at++];
				mesh.pages[p].parents.reserve(count);
				for (uint32_t i = 0; i < count; i++)
				{
					if (table[at] >= header.page_count)
						return false;
					mesh.pages[p].parents.push_back(table[at++]);
				}
			}
			mesh.parents_built = true;
		}
		else
		{
			// Older file: the parents come from the page bytes, which were read for that.
			build_page_parents(mesh);
			if (!keep_pages)
				for (Page& page : mesh.pages)
				{
					page.data.clear();
					page.data.shrink_to_fit();
				}
		}

		out = std::move(mesh);
		return true;
	}

	// Reads one page's bytes from the asset file the mesh came from and expands them.
	bool read_page(const Mesh& mesh, uint32_t page, std::vector<uint8_t>& out)
	{
		const Page& source = mesh.pages[page];
		if (!source.data.empty())
		{
			out = source.data;
			return true;
		}
		if (mesh.file.empty() || source.file_size == 0)
			return false;

		std::ifstream file(mesh.file, std::ios::binary);
		if (!file)
			return false;

		std::vector<uint8_t> stored(source.file_size);
		file.seekg(static_cast<std::streamoff>(source.file_offset));
		file.read(reinterpret_cast<char*>(stored.data()), stored.size());
		if (!file)
			return false;

		if (source.file_size == source.size)
		{
			out = std::move(stored);
			return true;
		}
		out.resize(source.size);
		return HAL::gdeflate_decompress(stored.data(), stored.size(), out.data(), out.size());
	}

	void read_stored_pages(const std::filesystem::path& path, std::span<PageFetch* const> fetches)
	{
		PROFILE(L"quanta_read_stored_pages");

		std::vector<PageFetch*> order(fetches.begin(), fetches.end());
		for (PageFetch* fetch : order)
			fetch->ok = false;

		// No stream buffer is set: a page is tens of KB, which the C runtime reads straight into the destination. A
		// pubsetbuf before open() is ignored by MSVC's filebuf (it needs an open file); if it did take effect, every
		// non-contiguous seek would drop and refill the whole buffer.
		std::ifstream file;
		{
			PROFILE(L"quanta_read_open");
			file.open(path, std::ios::binary);
		}
		if (!file)
			return;

		std::sort(order.begin(), order.end(), [](const PageFetch* a, const PageFetch* b)
		{
			return a->mesh->pages[a->page].file_offset < b->mesh->pages[b->page].file_offset;
		});

		// Each page is read straight into its own buffer, in file order; the seek is skipped when the stream already stands
		// at the page, so a contiguous chunk is one sequential pass.
		uint64_t position = std::numeric_limits<uint64_t>::max();
		for (PageFetch* fetch : order)
		{
			const Page& source = fetch->mesh->pages[fetch->page];
			if (source.file_size == 0)
				continue;

			fetch->bytes.resize(source.file_size);
			if (position != source.file_offset)
				file.seekg(static_cast<std::streamoff>(source.file_offset));
			file.read(reinterpret_cast<char*>(fetch->bytes.data()), fetch->bytes.size());
			if (!file)
			{
				file.clear();
				position = std::numeric_limits<uint64_t>::max();
				continue;
			}
			position = source.file_offset + source.file_size;
			fetch->ok = true;
		}
	}

	void expand_page(PageFetch& fetch)
	{
		const Page& source = fetch.mesh->pages[fetch.page];
		if (!fetch.ok || source.file_size == source.size)
			return;

		std::vector<uint8_t> expanded(source.size);
		fetch.ok = HAL::gdeflate_decompress(fetch.bytes.data(), fetch.bytes.size(), expanded.data(), expanded.size());
		fetch.bytes = std::move(expanded);
	}

	bool build_cut(const Mesh& mesh, uint32_t triangle_budget, CutGeometry& out)
	{
		constexpr float Infinity = std::numeric_limits<float>::max();
		// Clusters hold 100 triangles on average: enough to judge a budget without reading the pages.
		constexpr double TrianglesPerCluster = 100.0;

		out = {};
		if (mesh.groups.empty() || mesh.pages.empty())
			return false;

		const auto in_cut = [](const Group& group, float threshold) { return group.lod_error > threshold && group.refined_error <= threshold; };

		std::vector<float> candidates{ 0.0f };
		for (const Group& group : mesh.groups)
			if (group.lod_error != Infinity)
				candidates.push_back(group.lod_error);
		std::sort(candidates.begin(), candidates.end());
		candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());

		const auto estimate = [&](float threshold)
		{
			double clusters = 0;
			for (const Group& group : mesh.groups)
				if (in_cut(group, threshold))
					clusters += group.cluster_count;
			return clusters * TrianglesPerCluster;
		};

		// The estimate only falls as the threshold rises: the first candidate that fits is the finest cut that does.
		size_t low = 0, high = candidates.size() - 1;
		while (low < high)
		{
			const size_t middle = (low + high) / 2;
			if (estimate(candidates[middle]) <= triangle_budget)
				high = middle;
			else
				low = middle + 1;
		}
		const float threshold = candidates[low];
		out.error = threshold;

		std::map<uint32_t, std::vector<uint32_t>> by_page;
		for (uint32_t g = 0; g < mesh.groups.size(); g++)
			if (in_cut(mesh.groups[g], threshold))
				by_page[mesh.groups[g].page].push_back(g);

		struct VertexKey
		{
			std::array<uint32_t, sizeof(Vertex) / sizeof(uint32_t)> words;
			bool operator==(const VertexKey&) const = default;
		};
		struct VertexHash
		{
			size_t operator()(const VertexKey& key) const
			{
				uint64_t hash = 1469598103934665603ull;
				for (uint32_t word : key.words)
					hash = (hash ^ word) * 1099511628211ull;
				return static_cast<size_t>(hash);
			}
		};
		std::unordered_map<VertexKey, uint32_t, VertexHash> welded;

		std::vector<uint8_t> bytes;
		for (auto& [page, groups] : by_page)
		{
			if (!read_page(mesh, page, bytes))
				return false;

			for (uint32_t g : groups)
			{
				const Group& group = mesh.groups[g];
				const uint8_t* blob = bytes.data() + group.offset_in_page;
				const GroupHeader* header = reinterpret_cast<const GroupHeader*>(blob);
				const ClusterHeader* clusters = reinterpret_cast<const ClusterHeader*>(blob + sizeof(GroupHeader));

				for (uint32_t c = 0; c < header->cluster_count; c++)
				{
					const ClusterHeader& cluster = clusters[c];
					if (cluster.refined_error >= 0 && cluster.refined_error > threshold)
						continue;

					const Vertex* vertices = reinterpret_cast<const Vertex*>(blob + cluster.vertex_offset);
					const uint8_t* triangles = blob + cluster.triangle_offset;

					uint32_t remap[MaxClusterVertices];
					for (uint32_t v = 0; v < cluster.vertex_count; v++)
					{
						VertexKey key;
						std::memcpy(key.words.data(), &vertices[v], sizeof(Vertex));
						auto [it, added] = welded.try_emplace(key, static_cast<uint32_t>(out.vertices.size()));
						if (added)
							out.vertices.push_back(vertices[v]);
						remap[v] = it->second;
					}

					for (uint32_t t = 0; t < cluster.triangle_count; t++)
					{
						const uint32_t a = remap[triangles[t * 3]], b = remap[triangles[t * 3 + 1]], d = remap[triangles[t * 3 + 2]];
						if (a == b || b == d || a == d)
							continue;
						out.indices.insert(out.indices.end(), { a, b, d });
					}
				}
			}
		}

		return !out.indices.empty();
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

			{
				const uint32_t first = family_first(rec.family);
				const uint32_t count = family_count(rec.family);
				if (uint64_t(first) + count > mesh.groups.size() || g < first || g >= first + count)
					return name + " is not inside its own family";
				for (uint32_t m = first; m < first + count; m++)
					if (mesh.groups[m].lod_error != rec.lod_error || mesh.groups[m].family != rec.family)
						return name + " disagrees with its family";
			}

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
				else
				{
					if (rec.lod_error != Infinity && h.refined_error > rec.lod_error)
						return cname + " has a refined error above its group's (LOD error not monotonic)";

					const uint32_t first = family_first(h.refined_family);
					const uint32_t count = family_count(h.refined_family);
					if (uint64_t(first) + count > mesh.groups.size())
						return cname + " refines a family outside the mesh";
					for (uint32_t m = first; m < first + count; m++)
						if (mesh.groups[m].family != h.refined_family || mesh.groups[m].lod_error != h.refined_error)
							return cname + " refines a family whose LOD error is not its refined error";
				}
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
