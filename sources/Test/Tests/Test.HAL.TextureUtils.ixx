export module Test.HAL.TextureUtils;

export import Test.Framework;

import HAL;
import Core;

export namespace Test
{
	// Read a GPU texture back to CPU as texture_data (mip 0 of the specified subresource).
	inline HAL::texture_data::ptr readback_texture(HAL::TextureResource* tex, uint sub_resource = 0)
	{
		auto& device = HAL::Device::get();
		auto& tdesc  = tex->get_desc().as_texture();
		uint  width  = tdesc.Dimensions.x;
		uint  height = tdesc.Dimensions.y ? tdesc.Dimensions.y : 1;
		HAL::Format fmt = tdesc.Format;

		HAL::texture_data::ptr result;

		auto list   = device.get_upload_list();
		auto future = list->get_copy().read_texture(tex, sub_resource,
			[&](std::span<std::byte> data, HAL::texture_layout layout)
			{
				result = HAL::texture_data::from_readback(width, height, fmt, data, layout);
			});
		list->execute_and_wait();
		future.wait();

		return result;
	}

	// Compare tex against a saved PNG reference.
	//
	// reference_dir  – directory that holds reference PNGs  (default: "test_references")
	// results_dir    – directory for failure artefacts       (default: "test_results")
	// tolerance      – per-channel tolerance in [0,255]      (default: 0 = exact match)
	//
	// Behaviour:
	//   • No reference file  → saves current render as the reference and succeeds.
	//   • Reference exists   → compares pixel by pixel (RGBA8).
	//     On mismatch         → saves <name>_actual.png and <name>_diff.png to results_dir,
	//                           then throws TestFailure.
	inline void check_texture_reference(
		HAL::TextureResource*      tex,
		const std::string&         name,
		uint                       sub_resource  = 0,
		uint                       tolerance     = 0,
		const std::filesystem::path& reference_dir = "test_references",
		const std::filesystem::path& results_dir   = "test_results")
	{
		auto actual = readback_texture(tex, sub_resource);
		if (!actual)
			throw TestFailure("check_texture_reference: readback failed for '" + name + "'");

		auto actual_png = actual->to_png();
		if (actual_png.empty())
			throw TestFailure("check_texture_reference: PNG encoding failed for '" + name + "'");

		auto ref_path = reference_dir / (name + ".png");

		// ── No reference yet: save and pass ──────────────────────────────────────
		auto ref_file = FileSystem::get().get_file(ref_path);
		if (!ref_file)
		{
			std::string png_str(reinterpret_cast<const char*>(actual_png.data()), actual_png.size());
			FileSystem::get().save_data(ref_path, png_str);
			Log::get() << Log::LEVEL_INFO << "[TEXTURE] Saved new reference: " << ref_path.string() << Log::endl;
			return;
		}

		// ── Load and decode reference ─────────────────────────────────────────────
		auto ref_bytes = ref_file->load_all();
		auto reference = HAL::texture_data::from_png(ref_bytes.data(), ref_bytes.size());
		if (!reference)
			throw TestFailure("check_texture_reference: failed to decode reference PNG for '" + name + "'");

		// ── Decode actual back to RGBA8 so both sides are in the same space ───────
		auto actual_rgba = HAL::texture_data::from_png(actual_png.data(), actual_png.size());
		if (!actual_rgba)
			throw TestFailure("check_texture_reference: failed to decode actual PNG for '" + name + "'");

		if (actual_rgba->width != reference->width || actual_rgba->height != reference->height)
		{
			throw TestFailure("check_texture_reference: size mismatch for '" + name + "' "
				"(actual " + std::to_string(actual_rgba->width)  + "x" + std::to_string(actual_rgba->height) +
				" vs ref " + std::to_string(reference->width) + "x" + std::to_string(reference->height) + ")");
		}

		auto& act_data = actual_rgba->array[0]->mips[0]->data;
		auto& ref_data = reference->array[0]->mips[0]->data;

		// ── Pixel comparison ─────────────────────────────────────────────────────
		uint mismatch_pixels = 0;

		auto diff_td = std::make_shared<HAL::texture_data>(
			1, 1, actual_rgba->width, actual_rgba->height, 1, HAL::Format::R8G8B8A8_UNORM);
		auto& diff_data = diff_td->array[0]->mips[0]->data;

		size_t pixel_count = (size_t)actual_rgba->width * actual_rgba->height;
		for (size_t p = 0; p < pixel_count; ++p)
		{
			bool pixel_mismatch = false;
			for (int c = 0; c < 4; ++c)
			{
				size_t i = p * 4 + c;
				int d = std::abs((int)(uint8_t)act_data[i] - (int)(uint8_t)ref_data[i]);
				diff_data[i] = (uint8_t)std::min(255, d * 4);
				if (d > (int)tolerance)
					pixel_mismatch = true;
			}
			if (pixel_mismatch)
				++mismatch_pixels;
		}

		if (mismatch_pixels == 0)
			return;

		// ── Save artefacts on failure ─────────────────────────────────────────────
		auto save_png = [&](const std::filesystem::path& path, const std::vector<uint8_t>& png)
		{
			std::string s(reinterpret_cast<const char*>(png.data()), png.size());
			FileSystem::get().save_data(path, s);
		};

		save_png(results_dir / (name + "_actual.png"), actual_png);

		auto diff_png = diff_td->to_png();
		if (!diff_png.empty())
			save_png(results_dir / (name + "_diff.png"), diff_png);

		Log::get() << Log::LEVEL_ERROR
			<< "[TEXTURE] Mismatch '" << name << "': "
			<< mismatch_pixels << " / " << pixel_count << " pixels differ"
			<< " | actual → " << (results_dir / (name + "_actual.png")).string()
			<< " | diff → "   << (results_dir / (name + "_diff.png")).string()
			<< Log::endl;

		throw TestFailure("Texture mismatch: '" + name + "' ("
			+ std::to_string(mismatch_pixels) + "/" + std::to_string(pixel_count)
			+ " pixels differ, see " + results_dir.string() + "/)");
	}
}
