module Graphics:QuantaResidency;

import :QuantaResidency;

import Core;
import HAL;
import :Quanta;

import RenderSystem;

namespace Quanta
{
	Residency::Residency()
		: pages(RenderSystem::get().device(), MaxPageDwords)
		, groups(RenderSystem::get().device(), MaxGroupDwords)
		, nodes(RenderSystem::get().device(), MaxNodeDwords)
	{
	}

	Resident::~Resident()
	{
		if (!Residency::is_good())
			return;

		auto& residency = Residency::get();
		residency.pages.free(page_handle);
		residency.groups.free(group_handle);
		residency.nodes.free(node_handle);
	}

	std::shared_ptr<Resident> Residency::acquire(const std::shared_ptr<Mesh>& mesh)
	{
		std::lock_guard<std::mutex> g(m);

		if (auto it = residents.find(mesh.get()); it != residents.end())
			if (auto existing = it->second.lock())
				return existing;

		auto resident = std::make_shared<Resident>();
		resident->mesh = mesh;

		pages.allocate(resident->page_handle, mesh->pages.size() * PageDwords);
		groups.allocate(resident->group_handle, mesh->groups.size() * GroupDwords);
		nodes.allocate(resident->node_handle, mesh->nodes.size() * NodeDwords);

		const uint64_t page_base = resident->page_handle.get_offset();
		const uint64_t group_base = resident->group_handle.get_offset() / GroupDwords;
		ASSERT(resident->group_handle.get_offset() % GroupDwords == 0 && resident->node_handle.get_offset() % NodeDwords == 0);
		ASSERT(page_base + mesh->pages.size() * PageDwords <= 0xFFFFFFFFull);

		for (size_t p = 0; p < mesh->pages.size(); p++)
		{
			const auto& data = mesh->pages[p].data;
			std::vector<uint> dwords((data.size() + 3) / 4, 0);
			std::memcpy(dwords.data(), data.data(), data.size());
			pages.write(page_base + p * PageDwords, dwords);
		}

		std::vector<uint> group_dwords(mesh->groups.size() * GroupDwords);
		for (size_t i = 0; i < mesh->groups.size(); i++)
		{
			Group record = mesh->groups[i];
			record.page = static_cast<uint32_t>(page_base + uint64_t(record.page) * PageDwords);
			std::memcpy(&group_dwords[i * GroupDwords], &record, sizeof(Group));
		}
		groups.write(resident->group_handle.get_offset(), group_dwords);

		std::vector<uint> node_dwords(mesh->nodes.size() * NodeDwords);
		for (size_t i = 0; i < mesh->nodes.size(); i++)
		{
			Node record = mesh->nodes[i];
			record.first_group = static_cast<uint32_t>(group_base + record.first_group);
			std::memcpy(&node_dwords[i * NodeDwords], &record, sizeof(Node));
		}
		nodes.write(resident->node_handle.get_offset(), node_dwords);

		residents[mesh.get()] = resident;
		return resident;
	}

	std::shared_ptr<Resident> Residency::pin(const std::shared_ptr<Mesh>& mesh)
	{
		{
			std::lock_guard<std::mutex> g(m);
			if (auto it = pinned.find(mesh.get()); it != pinned.end())
				return it->second;
		}

		auto resident = acquire(mesh);

		std::lock_guard<std::mutex> g(m);
		pinned[mesh.get()] = resident;
		return resident;
	}

	void Residency::prepare(HAL::CommandList::ptr& list)
	{
		pages.prepare(list);
		groups.prepare(list);
		nodes.prepare(list);
	}
}
