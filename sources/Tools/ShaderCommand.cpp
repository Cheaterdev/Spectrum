#include "Commands.h"

import Core;
import HAL;
import RenderSystem;

// spectrum_tool shaders [--target android|all] [--out <dir>]
//
// Everything is compiled regardless of target: creating the device builds every
// PSO in every key combination, and PSOs this GPU can't run still get their
// shaders compiled (PSOBuildObserver::compile_unsupported). The target only
// decides which cache files are shipped -- <out>/<backend>/<crc32(header)>.bin,
// the same files the engine's shader cache reads at runtime -- and what the
// manifest marks as included.

namespace
{
	struct StageRecord
	{
		std::string stage;
		HAL::shader_header header;
		std::string hash;
	};

	struct PSORecord
	{
		std::string name;
		std::string kind;
		PSOBuildResult result;
		bool requires_raytracing = false;
		bool requires_tiled_depth = false;
		bool uses_mesh_shaders = false;
		std::vector<StageRecord> stages;
	};

	struct TargetProfile
	{
		std::string name;
		bool mesh_shaders;
		bool raytracing;
		bool tiled_depth;

		// Empty when the PSO is wanted on this target, otherwise why not.
		std::string exclusion(const PSORecord& pso) const
		{
			if (pso.result == PSOBuildResult::Failed)
				return "compile failed";
			if (pso.uses_mesh_shaders && !mesh_shaders)
				return "mesh shaders";
			if (pso.requires_raytracing && !raytracing)
				return "raytracing";
			if (pso.requires_tiled_depth && !tiled_depth)
				return "tiled depth";
			return {};
		}
	};

	std::optional<TargetProfile> find_target(const std::string& name)
	{
		if (name == "all")
			return TargetProfile{ "all", true, true, true };
		// Ray queries are optional on Android but present on current Adreno (830 reports them), and
		// PSO::create skips them on devices without; mesh shaders are not available on any
		// Android GPU the port targets. Both are still gated at runtime by the device's own caps.
		if (name == "android")
			return TargetProfile{ "android", false, true, false };
		return std::nullopt;
	}

	std::string json_escape(const std::string& s)
	{
		std::string out;
		for (char c : s)
		{
			switch (c)
			{
			case '"': out += "\\\""; break;
			case '\\': out += "\\\\"; break;
			case '\n': out += "\\n"; break;
			case '\r': out += "\\r"; break;
			case '\t': out += "\\t"; break;
			default: out += c;
			}
		}
		return out;
	}

	const char* result_name(PSOBuildResult r)
	{
		switch (r)
		{
		case PSOBuildResult::Created: return "created";
		case PSOBuildResult::Unsupported: return "unsupported_on_build_device";
		default: return "failed";
		}
	}

	void add_stage(PSORecord& pso, const char* stage, const HAL::shader_header& header)
	{
		if (header.entry_point.empty())
			return;
		pso.stages.push_back({ stage, header, Hasher::hash(header) });
	}
}

int command_shaders(const CommandArgs& args)
{
	std::string target_name = "all";
	std::filesystem::path out_dir = "shader_cache";

	for (size_t i = 0; i < args.size(); ++i)
	{
		if (args[i] == "--target" && i + 1 < args.size())
			target_name = args[++i];
		else if (args[i] == "--out" && i + 1 < args.size())
			out_dir = args[++i];
		else
		{
			std::printf("shaders: unknown argument '%s'\n", args[i].c_str());
			return 1;
		}
	}

	auto target = find_target(target_name);
	if (!target)
	{
		std::printf("shaders: unknown target '%s' (android, all)\n", target_name.c_str());
		return 1;
	}

	std::mutex records_mutex;
	std::vector<PSORecord> records;

	PSOBuildObserver::compile_unsupported() = true;

	PSOBuildObserver::compute() = [&](const SimpleComputePSO& pso, PSOBuildResult result)
	{
		PSORecord r;
		r.name = pso.name;
		r.kind = "compute";
		r.result = result;
		r.requires_raytracing = pso.requires_raytracing;
		r.requires_tiled_depth = pso.requires_tiled_depth;
		add_stage(r, "compute", pso.compute);

		std::lock_guard lock(records_mutex);
		records.push_back(std::move(r));
	};

	PSOBuildObserver::graphics() = [&](const SimpleGraphicsPSO& pso, PSOBuildResult result)
	{
		PSORecord r;
		r.name = pso.name;
		r.kind = "graphics";
		r.result = result;
		r.requires_raytracing = pso.requires_raytracing;
		r.requires_tiled_depth = pso.requires_tiled_depth;
		r.uses_mesh_shaders = !pso.mesh.entry_point.empty() || !pso.amplification.entry_point.empty();
		add_stage(r, "vertex", pso.vertex);
		add_stage(r, "pixel", pso.pixel);
		add_stage(r, "geometry", pso.geometry);
		add_stage(r, "hull", pso.hull);
		add_stage(r, "domain", pso.domain);
		add_stage(r, "mesh", pso.mesh);
		add_stage(r, "amplification", pso.amplification);

		std::lock_guard lock(records_mutex);
		records.push_back(std::move(r));
	};

	if (!RenderSystem::create())
	{
		std::printf("shaders: no suitable GPU device (needed to build the PSOs)\n");
		return 2;
	}

	// Creating the device built every PSO; the observer has seen them all.
	RenderSystem::get().device().wait_idle();
	RenderSystem::reset();

	PSOBuildObserver::compute() = nullptr;
	PSOBuildObserver::graphics() = nullptr;

	const std::string backend = HAL::get_backend_name();
	const std::filesystem::path cache_dir = std::filesystem::path("cache") / backend;
	const std::filesystem::path dest_dir = out_dir / backend;

	std::error_code ec;
	std::filesystem::create_directories(dest_dir, ec);

	std::sort(records.begin(), records.end(), [](const PSORecord& a, const PSORecord& b) {
		return std::tie(a.name, a.kind) < std::tie(b.name, b.kind);
	});

	size_t pso_included = 0, pso_excluded = 0, failed = 0, missing = 0;
	std::set<std::string> shipped, all_hashes;
	std::string manifest = "{\n  \"backend\": \"" + backend + "\",\n  \"target\": \"" + target->name + "\",\n  \"psos\": [\n";

	for (size_t i = 0; i < records.size(); ++i)
	{
		auto& pso = records[i];
		const std::string reason = target->exclusion(pso);
		if (pso.result == PSOBuildResult::Failed)
			++failed;

		if (reason.empty())
		{
			++pso_included;
			for (auto& s : pso.stages)
			{
				if (!shipped.insert(s.hash).second)
					continue;
				const auto name = s.hash + ".bin";
				if (!std::filesystem::exists(cache_dir / name))
				{
					Log::get() << Log::LEVEL_ERROR << "shaders: " << pso.name << " " << s.stage << " compiled but " << (cache_dir / name).string() << " is missing" << Log::endl;
					++missing;
					continue;
				}
				std::filesystem::copy_file(cache_dir / name, dest_dir / name, std::filesystem::copy_options::overwrite_existing, ec);
			}
		}
		else
			++pso_excluded;

		for (auto& s : pso.stages)
			all_hashes.insert(s.hash);

		manifest += "    { \"name\": \"" + json_escape(pso.name) + "\", \"kind\": \"" + pso.kind + "\", \"build\": \"" + result_name(pso.result) + "\""
			+ ", \"requires_raytracing\": " + (pso.requires_raytracing ? "true" : "false")
			+ ", \"requires_tiled_depth\": " + (pso.requires_tiled_depth ? "true" : "false")
			+ ", \"mesh_shaders\": " + (pso.uses_mesh_shaders ? "true" : "false")
			+ ", \"included\": " + (reason.empty() ? "true" : "false")
			+ ", \"excluded_reason\": \"" + json_escape(reason) + "\",\n      \"stages\": [\n";

		for (size_t j = 0; j < pso.stages.size(); ++j)
		{
			auto& s = pso.stages[j];
			manifest += "        { \"stage\": \"" + s.stage + "\", \"file\": \"" + json_escape(s.header.file_name) + "\", \"entry\": \"" + json_escape(s.header.entry_point)
				+ "\", \"hash\": \"" + s.hash + "\", \"macros\": [";
			for (size_t k = 0; k < s.header.macros.size(); ++k)
				manifest += std::string(k ? ", " : "") + "\"" + json_escape(s.header.macros[k].name) + "=" + json_escape(s.header.macros[k].value) + "\"";
			manifest += std::string("] }") + (j + 1 < pso.stages.size() ? "," : "") + "\n";
		}
		manifest += std::string("      ] }") + (i + 1 < records.size() ? "," : "") + "\n";
	}
	manifest += "  ]\n}\n";

	{
		std::ofstream f(out_dir / "shaders.manifest.json", std::ios::binary);
		f << manifest;
	}

	std::printf("\nshaders: backend %s, target %s\n", backend.c_str(), target->name.c_str());
	std::printf("  PSOs built (all keys): %zu, shipped: %zu, excluded for target: %zu, failed: %zu\n", records.size(), pso_included, pso_excluded, failed);
	std::printf("  shader blobs: %zu distinct, %zu shipped, %zu missing from cache\n", all_hashes.size(), shipped.size() - missing, missing);
	std::printf("  output: %s\n", std::filesystem::absolute(out_dir).string().c_str());

	return (failed || missing) ? 1 : 0;
}
