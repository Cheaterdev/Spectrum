module Test.HAL.TextureUtils;
import RenderSystem;

import HAL;
import Core;

namespace Test
{
	HAL::texture_data::ptr readback_texture(HAL::TextureResource* tex, uint sub_resource)
	{
		auto& device = RenderSystem::get().device();
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

	void check_texture_reference(
		HAL::TextureResource*        tex,
		const std::string&           name,
		uint                         sub_resource,
		uint                         tolerance,
		const std::filesystem::path& reference_dir,
		const std::filesystem::path& results_dir)
	{
		auto actual = readback_texture(tex, sub_resource);
		if (!actual)
			throw TestFailure("check_texture_reference: readback failed for '" + name + "'");

		auto actual_png = actual->to_png();
		if (actual_png.empty())
			throw TestFailure("check_texture_reference: PNG encoding failed for '" + name + "'");

		auto ref_path = reference_dir / (name + ".png");

		// No reference yet: save and pass
		auto ref_file = FileSystem::get().get_file(ref_path);
		if (!ref_file)
		{
			std::string png_str(reinterpret_cast<const char*>(actual_png.data()), actual_png.size());
			FileSystem::get().save_data(ref_path, png_str);
			Log::get() << Log::LEVEL_INFO << "[TEXTURE] Saved new reference: " << ref_path.string() << Log::endl;
			return;
		}

		// Load and decode reference
		auto ref_bytes = ref_file->load_all();
		auto reference = HAL::texture_data::from_png(ref_bytes.data(), ref_bytes.size());
		if (!reference)
			throw TestFailure("check_texture_reference: failed to decode reference PNG for '" + name + "'");

		// Decode actual back to RGBA8 so both sides are in the same space
		auto actual_rgba = HAL::texture_data::from_png(actual_png.data(), actual_png.size());
		if (!actual_rgba)
			throw TestFailure("check_texture_reference: failed to decode actual PNG for '" + name + "'");

		if (actual_rgba->width != reference->width || actual_rgba->height != reference->height)
		{
			throw TestFailure("check_texture_reference: size mismatch for '" + name + "' "
				"(actual " + std::to_string(actual_rgba->width) + "x" + std::to_string(actual_rgba->height) +
				" vs ref "  + std::to_string(reference->width)  + "x" + std::to_string(reference->height)  + ")");
		}

		auto& act_data = actual_rgba->array[0]->mips[0]->data;
		auto& ref_data = reference->array[0]->mips[0]->data;

		uint mismatch_pixels = 0;

		auto diff_td = std::make_shared<HAL::texture_data>(
			1, 1, actual_rgba->width, actual_rgba->height, 1, HAL::Format::R8G8B8A8_UNORM);
		auto& diff_data = diff_td->array[0]->mips[0]->data;

		size_t pixel_count = (size_t)actual_rgba->width * actual_rgba->height;
		// Bytes per pixel may differ: e.g. actual is RGBA8 (4) but an old reference
		// was saved as greyscale R8 (1). Use separate strides per side and compare
		// only the channels present on both (capped at 3 to skip alpha).
		size_t bpp_act = act_data.size() / pixel_count;
		size_t bpp_ref = ref_data.size() / pixel_count;
		size_t ch      = std::min({ bpp_act, bpp_ref, (size_t)3 });

		for (size_t p = 0; p < pixel_count; ++p)
		{
			int max_diff = 0;
			bool pixel_mismatch = false;
			for (size_t c = 0; c < ch; ++c)
			{
				int d = std::abs((int)(uint8_t)act_data[p * bpp_act + c]
				               - (int)(uint8_t)ref_data[p * bpp_ref + c]);
				if (d > max_diff) max_diff = d;
				if (d > (int)tolerance)
					pixel_mismatch = true;
			}
			uint8_t brightness = (uint8_t)std::min(255, max_diff * 4);
			diff_data[p * 4 + 0] = brightness;
			diff_data[p * 4 + 1] = brightness;
			diff_data[p * 4 + 2] = brightness;
			diff_data[p * 4 + 3] = 255;
			if (pixel_mismatch)
				++mismatch_pixels;
		}

		if (mismatch_pixels == 0)
			return;

		// Save artefacts on failure
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
