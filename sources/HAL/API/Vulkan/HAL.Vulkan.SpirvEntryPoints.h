#pragma once
// The entry points of a SPIR-V module: name and execution model (OpEntryPoint).
// A DXR library compiled to SPIR-V carries one per raygen/miss/hit shader; the
// ray tracing pipeline needs each one's stage, which nothing else records.

// Included after `import stl.core;` — the standard library comes from the module, not from headers here.

namespace HAL::Spirv
{
	// SPIR-V ExecutionModel values (KHR ray tracing).
	enum : uint32_t
	{
		RayGeneration = 5313,
		Intersection  = 5314,
		AnyHit        = 5315,
		ClosestHit    = 5316,
		Miss          = 5317,
		Callable      = 5318,
	};

	struct EntryPoint
	{
		uint32_t    execution_model = 0;
		uint32_t    function_id     = 0;   // result id of its OpFunction
		std::string name;
	};

	inline std::vector<EntryPoint> entry_points(const void* data, size_t size)
	{
		std::vector<EntryPoint> result;
		if (size < 20 || size % 4) return result;

		const uint32_t* words = static_cast<const uint32_t*>(data);
		const size_t    count = size / 4;
		if (words[0] != 0x07230203u) return result;   // SPIR-V magic

		constexpr uint32_t OpEntryPoint = 15;
		constexpr uint32_t OpFunction   = 54;          // entry points all precede the first function

		for (size_t i = 5; i < count;)
		{
			const uint32_t op  = words[i] & 0xFFFFu;
			const uint32_t len = words[i] >> 16;
			if (len == 0 || i + len > count || op == OpFunction) break;

			if (op == OpEntryPoint && len >= 4)
			{
				// Literal string: null-terminated, packed into the words after <model, id>.
				const char*  str = reinterpret_cast<const char*>(words + i + 3);
				const size_t max = (len - 3) * 4;
				result.push_back({ words[i + 1], words[i + 2], std::string(str, strnlen(str, max)) });
			}
			i += len;
		}
		return result;
	}
}
