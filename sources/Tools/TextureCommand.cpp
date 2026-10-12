#include "Commands.h"

import Core;
import HAL;
import RenderSystem;
import Graphics;

// spectrum_tool textures [--dir <rel dir>]... [--out <dir>]
//
// Loads every image under each --dir (default textures/gui) through the engine's own texture
// path, which decodes it and writes the cache blob cache/<backend>/<crc32(header)>.bin (raw GPU
// texel data on Vulkan). The blobs are copied to <out>/<backend>/ next to the shader blobs: the
// device has no image decoders (DirectXTex/WIC are Windows-only) and loads these instead.
//
// The header must be exactly what the runtime asks for. EngineAsset<TextureAsset> builds
// texure_header(path, force_srgb = true), so that is what is baked.

namespace
{
	bool is_image(const std::filesystem::path& p)
	{
		const auto ext = to_lower(p.extension().string());
		return ext == ".png" || ext == ".dds" || ext == ".tga" || ext == ".jpg";
	}

	std::string json_escape(const std::string& s)
	{
		std::string out;
		for (char c : s)
		{
			if (c == '"' || c == '\\')
				out += '\\';
			out += c;
		}
		return out;
	}
}

int command_textures(const CommandArgs& args)
{
	std::vector<std::string> dirs;
	std::filesystem::path out_dir = "shader_cache";

	for (size_t i = 0; i < args.size(); ++i)
	{
		if (args[i] == "--dir" && i + 1 < args.size())
			dirs.push_back(args[++i]);
		else if (args[i] == "--out" && i + 1 < args.size())
			out_dir = args[++i];
		else
		{
			std::printf("textures: unknown argument '%s'\n", args[i].c_str());
			return 1;
		}
	}
	if (dirs.empty())
		dirs.push_back("textures/gui");

	if (!RenderSystem::create())
	{
		std::printf("textures: no suitable GPU device\n");
		return 2;
	}

	const std::string backend = HAL::get_backend_name();
	const std::filesystem::path cache_dir = std::filesystem::path("cache") / backend;
	const std::filesystem::path dest_dir = out_dir / backend;
	std::error_code ec;
	std::filesystem::create_directories(dest_dir, ec);

	size_t loaded = 0, failed = 0, missing = 0;
	std::string manifest = "{\n  \"backend\": \"" + backend + "\",\n  \"textures\": [\n";

	for (const auto& dir : dirs)
	{
		std::vector<std::string> files;
		for (auto& entry : std::filesystem::directory_iterator(dir, ec))
			if (entry.is_regular_file() && is_image(entry.path()))
				files.push_back(dir + "/" + entry.path().filename().string());
		std::sort(files.begin(), files.end());

		for (const auto& file : files)
		{
			const HAL::texure_header header(to_path(file), true);
			const std::string hash = Hasher::hash(header);

			auto texture = HAL::Texture::get_resource(header);
			if (!texture || texture == HAL::Texture::null)
			{
				Log::get() << Log::LEVEL_ERROR << "textures: failed to load " << file << Log::endl;
				++failed;
				continue;
			}

			const auto name = hash + ".bin";
			if (!std::filesystem::exists(cache_dir / name))
			{
				Log::get() << Log::LEVEL_ERROR << "textures: " << file << " loaded but " << (cache_dir / name).string() << " is missing" << Log::endl;
				++missing;
				continue;
			}
			std::filesystem::copy_file(cache_dir / name, dest_dir / name, std::filesystem::copy_options::overwrite_existing, ec);

			manifest += std::string(loaded ? ",\n" : "") + "    { \"file\": \"" + json_escape(file) + "\", \"hash\": \"" + hash + "\" }";
			++loaded;
		}
	}
	manifest += "\n  ]\n}\n";

	{
		std::ofstream f(out_dir / "textures.manifest.json", std::ios::binary);
		f << manifest;
	}

	// The texture manager keeps every loaded texture alive; release them before the device goes.
	HAL::Texture::reset_manager();
	RenderSystem::get().device().wait_idle();
	RenderSystem::reset();

	std::printf("\ntextures: backend %s, %zu baked, %zu failed, %zu missing from cache\n", backend.c_str(), loaded, failed, missing);
	std::printf("  output: %s\n", std::filesystem::absolute(out_dir).string().c_str());
	return (failed || missing) ? 1 : 0;
}
