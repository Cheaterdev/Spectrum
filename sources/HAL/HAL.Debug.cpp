module HAL:Debug;
import Core;
import :Types;

namespace HAL::Debug
{
	namespace
	{
		struct BreakKeyHash
		{
			size_t operator()(const BreakKey& k) const
			{
				auto combine = [](size_t h, size_t v) -> size_t {
					return h ^ (v * 2654435761u + 0x9e3779b9u + (h << 6) + (h >> 2));
				};
				size_t h = std::hash<std::string>{}(k.resource_name);
				h = combine(h, std::hash<uint>{}(k.subres));
				h = combine(h, static_cast<size_t>(k.before.operation));
				h = combine(h, static_cast<size_t>(k.before.access));
				h = combine(h, static_cast<size_t>(k.before.layout));
				h = combine(h, static_cast<size_t>(k.after.operation));
				h = combine(h, static_cast<size_t>(k.after.access));
				h = combine(h, static_cast<size_t>(k.after.layout));
				return h;
			}
		};

		std::mutex                                     s_mutex;
		std::unordered_set<BreakKey, BreakKeyHash>     s_breaks;
	}

	bool BarrierBreakpoints::toggle(const BreakKey& key)
	{
		std::lock_guard lock(s_mutex);
		auto it = s_breaks.find(key);
		if (it != s_breaks.end())
		{
			s_breaks.erase(it);
			return false;
		}
		s_breaks.insert(key);
		return true;
	}

	bool BarrierBreakpoints::has(const BreakKey& key)
	{
		std::lock_guard lock(s_mutex);
		return s_breaks.count(key) > 0;
	}

	void BarrierBreakpoints::check_barrier(std::string_view resource, uint subres,
	                                        const ResourceState& before, const ResourceState& after)
	{
		BreakKey key{ std::string(resource), subres, before, after };
		if (has(key))
			__debugbreak();
	}

	void BarrierBreakpoints::check_usage(std::string_view resource, uint subres,
	                                      const ResourceState& wanted)
	{
		// Linear scan — typically 0-2 active breakpoints, so O(n) is acceptable.
		std::lock_guard lock(s_mutex);
		for (const auto& k : s_breaks)
		{
			if (k.subres == subres && k.after == wanted &&
			    k.resource_name == resource)
			{
				__debugbreak();
				return;
			}
		}
	}
}
