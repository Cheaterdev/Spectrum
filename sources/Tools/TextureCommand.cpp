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
	std::vector<std::string> dirs, explicit_files;
	std::filesystem::path out_dir = "shader_cache";

	for (size_t i = 0; i < args.size(); ++i)
	{
		if (args[i] == "--dir" && i + 1 < args.size())
			dirs.push_back(args[++i]);
		else if (args[i] == "--file" && i + 1 < args.size())
			explicit_files.push_back(args[++i]);
		else if (args[i] == "--out" && i + 1 < args.size())
			out_dir = args[++i];
		else
		{
			std::printf("textures: unknown argument '%s'\n", args[i].c_str());
			return 1;
		}
	}
	if (dirs.empty() && explicit_files.empty())
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

	// One group per --dir (its images, sorted) plus a last one holding the --file entries.
	std::vector<std::vector<std::string>> groups;
	for (const auto& dir : dirs)
	{
		std::vector<std::string> files;
		for (auto& entry : std::filesystem::directory_iterator(dir, ec))
			if (entry.is_regular_file() && is_image(entry.path()))
				files.push_back(dir + "/" + entry.path().filename().string());
		std::sort(files.begin(), files.end());
		groups.push_back(std::move(files));
	}
	groups.push_back(explicit_files);

	for (const auto& files : groups)
	{
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

// spectrum_tool assets [--out <dir>]
//
// Saves every EngineAsset (the GUI skin, the sky lookup textures, best-fit normals, ...) as a real asset
// file, assets/<Type>/<name>_<guid>.asset: the asset system's own format, with the texture data inside.
// EngineAsset::get_asset() asks the AssetManager for its (name-derived) id before it falls back to the
// loader that decodes an image, so a device that ships these files needs no image decoder and no list of
// textures kept by hand. The files are copied to <out>/assets/.
int command_assets(const CommandArgs& args)
{
	std::filesystem::path out_dir = "shader_cache";
	for (size_t i = 0; i < args.size(); ++i)
	{
		if (args[i] == "--out" && i + 1 < args.size())
			out_dir = args[++i];
		else
		{
			std::printf("assets: unknown argument '%s'\n", args[i].c_str());
			return 1;
		}
	}

	if (!RenderSystem::create())
	{
		std::printf("assets: no suitable GPU device\n");
		return 2;
	}

	// A previous run's files must not leak into this one (or stay behind for the desktop app to pick up).
	std::error_code ec;
	std::filesystem::remove_all("assets", ec);

	// Materials also build the vertex-stage GBuffer pipeline, so the assets carry it for devices without mesh shaders.
	materials::PipelineOptions::vertex_front_end() = true;

	preload_engine_assets();   // runs every registered loader; each registers its asset with the AssetManager

	// The Device target: assets are saved without editing data (a material's graph) and with their compiled
	// shaders inside, so the phone needs no source files and no compiler. Textures, meshes and materials;
	// the shader-include text assets are editing inputs and stay out.
	AssetSerialization::target() = AssetTarget::Device;
	ShaderEmbedding::enabled() = true;

	size_t saved = 0;
	for (auto& [id, storage] : AssetManager::get().get_assets())
	{
		auto asset = storage->get_asset();
		if (!asset)
			continue;

		const auto type = asset->get_type();
		if (type == Asset_Type::MATERIAL)
		{
			if (auto material = asset->get_ptr<materials::universal_material>())
			{
				material->drop_editor_references();

				// Every pipeline permutation has to be compiled (and so in the shader cache) before the
				// material is written with it.
				for (int i = 0; i < 6000 && !material->is_ready_to_draw(); ++i)
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
				if (!material->is_ready_to_draw())
					Log::get() << Log::LEVEL_ERROR << "assets: pipelines of " << convert(storage->get_name()) << " did not finish compiling" << Log::endl;
			}
		}
		else if (type != Asset_Type::TEXTURE && type != Asset_Type::MESH)
			continue;

		storage->save();
		++saved;
	}

	const std::filesystem::path dest = out_dir / "assets";
	std::filesystem::remove_all(dest, ec);
	std::filesystem::create_directories(dest, ec);
	std::filesystem::copy("assets", dest, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing, ec);
	std::filesystem::remove_all("assets", ec);

	HAL::Texture::reset_manager();
	RenderSystem::get().device().wait_idle();
	RenderSystem::reset();

	std::printf("\nassets: %zu saved, copied to %s%s\n", saved, std::filesystem::absolute(dest).string().c_str(), ec ? " (COPY FAILED)" : "");
	return (saved && !ec) ? 0 : 1;
}
