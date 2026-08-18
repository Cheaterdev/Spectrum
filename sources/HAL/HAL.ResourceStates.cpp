module HAL:ResourceStates;

import Core;

import HAL;
namespace HAL
{

	// --- free functions ---

	TextureLayout resting_layout(const Resource* resource)
	{
		auto flags = resource->get_desc().Flags;

		// A back buffer rests ready to present, not in a read layout -- and
		// transition_present() records exactly that as its last use, so
		// anything else here would emit a barrier undoing it.
		if (check(flags & ResFlags::Swapchain))       return TextureLayout::PRESENT;

		if (check(flags & ResFlags::ShaderResource))  return TextureLayout::SHADER_RESOURCE;
		if (check(flags & ResFlags::UnorderedAccess)) return TextureLayout::UNORDERED_ACCESS;

		// Nothing declares it readable -- leave it where an upload would put it.
		return TextureLayout::COPY_DEST;
	}

	bool IsCompatible(CommandListType a, CommandListType b)
	{
		if (a == CommandListType::DIRECT) return true;
		if (b == CommandListType::DIRECT) return false;

		if (a == CommandListType::COMPUTE) return true;
		if (b == CommandListType::COMPUTE) return false;

		return true;
	}

	bool IsFullySupport(CommandListType type, const ResourceState& states)
	{
		return IsCompatible(type, states.get_best_cmd_type());
	}

	CommandListType Merge(CommandListType a, CommandListType b)
	{
		if (a == CommandListType::DIRECT || b == CommandListType::DIRECT) return CommandListType::DIRECT;
		if (a == CommandListType::COMPUTE || b == CommandListType::COMPUTE) return CommandListType::COMPUTE;
		return CommandListType::COPY;
	}

	std::optional<ResourceState> merge_state(const ResourceState& source, const ResourceState& need)
	{
		if (source == ResourceStates::UNKNOWN)
			return std::nullopt;

		if (source.has_write_bits() || need.has_write_bits())
			return std::nullopt;

		if (source.is_no_access() || need.is_no_access())
			return std::nullopt;

		auto res = source | need;
		return res;
	}

	std::optional<TextureLayout> merge_layout(const TextureLayout& source, const TextureLayout& need)
	{
		if (source == TextureLayout::NONE) return need;
		if (source == need) return need;
		if (source == TextureLayout::UNDEFINED) return need;

		if (need == TextureLayout::UNDEFINED) return need;		  // for alias end

		static const TextureLayout LAYOUT_WRITE = TextureLayout::UNORDERED_ACCESS | TextureLayout::DEPTH_STENCIL_WRITE | TextureLayout::RENDER_TARGET | TextureLayout::COPY_DEST;
		if (check(source & LAYOUT_WRITE) || check(need & LAYOUT_WRITE))
			return std::nullopt;

		// PRESENT can't merge with any other layout (unlike SHADER_RESOURCE|
		// COPY_SOURCE) — OR-ing it in produces a bit combination to_native()
		// has no case for and asserts on. nullopt forces a real separate
		// transition instead.
		if (check(source & TextureLayout::PRESENT) || check(need & TextureLayout::PRESENT))
			return std::nullopt;

		return source | need;
	}


	// --- Barriers ---

	Barriers::Barriers(CommandListType type) : type(type)
	{
	}

	Barriers::operator bool() const
	{
		return !barriers.empty();
	}

	void Barriers::clear()
	{
		barriers.clear();
	}

	const std::vector<Barrier>& Barriers::get_barriers() const
	{
		return barriers;
	}

	void Barriers::transition(const Resource* resource, ResourceState before, ResourceState after, UINT subres, BarrierFlags flags)
	{
		ASSERT(resource);

		ASSERT(IsFullySupport(type, before));
		ASSERT(IsFullySupport(type, after));

		if (check(after.access & (BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_WRITE | BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_READ)))
			ASSERT(check(resource->get_desc().Flags & ResFlags::Raytracing));

		barriers.emplace_back(Barrier{ const_cast<Resource*>(resource), before, after, subres, flags });
	}


}
