module Test.HAL.TextureUtils;
import RenderSystem;

import HAL;
import Core;

namespace
{
	float half_to_float(uint16_t h)
	{
		uint32_t sign = (uint32_t)(h & 0x8000) << 16;
		uint32_t exp  = (h >> 10) & 0x1f;
		uint32_t mant = h & 0x3ff;
		uint32_t bits;
		if (exp == 0)
		{
			if (mant == 0)
				bits = sign;
			else
			{
				// subnormal: renormalize into a float32 normal
				exp = 127 - 15 + 1;
				while (!(mant & 0x400)) { mant <<= 1; --exp; }
				bits = sign | (exp << 23) | ((mant & 0x3ff) << 13);
			}
		}
		else if (exp == 0x1f)
			bits = sign | 0x7f800000 | (mant << 13);
		else
			bits = sign | ((exp + 127 - 15) << 23) | (mant << 13);
		return std::bit_cast<float>(bits);
	}

	uint8_t unorm8(float c)
	{
		return (uint8_t)std::lround(std::clamp(c, 0.0f, 1.0f) * 255.0f);
	}

	uint8_t linear_to_srgb8(float c)
	{
		c = std::clamp(c, 0.0f, 1.0f);
		return unorm8(c <= 0.0031308f ? c * 12.92f : 1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f);
	}

	// UI PSOs target the scRGB swapchain (R16G16B16A16_FLOAT, linear). With
	// ui_scale = 1 the shader writes srgb_to_linear(color), so re-encoding to
	// sRGB here reproduces the 8-bit values the golden references hold.
	HAL::texture_data::ptr fp16_to_rgba8(const HAL::texture_data& src, Test::FloatEncoding encoding)
	{
		auto encode = encoding == Test::FloatEncoding::Linear ? linear_to_srgb8 : unorm8;

		auto& in  = src.array[0]->mips[0];
		auto  out = std::make_shared<HAL::texture_data>(1, 1, in->width, in->height, 1, HAL::Format::R8G8B8A8_UNORM);
		auto& dst = out->array[0]->mips[0];

		for (uint y = 0; y < in->height; ++y)
		{
			auto src_row = reinterpret_cast<const uint16_t*>(in->data.data() + (size_t)y * in->width_stride);
			auto dst_row = dst->data.data() + (size_t)y * dst->width_stride;
			for (uint x = 0; x < in->width; ++x)
			{
				for (uint c = 0; c < 3; ++c)
					dst_row[x * 4 + c] = encode(half_to_float(src_row[x * 4 + c]));
				dst_row[x * 4 + 3] = unorm8(half_to_float(src_row[x * 4 + 3]));
			}
		}
		return out;
	}
}

namespace Test
{
	void set_ui_target(HAL::CommandList::ptr& list, HAL::Texture2DView& view, vec4 srgb_clear)
	{
		auto to_linear = [](float c) { return c <= 0.04045f ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f); };

		HAL::CompiledRT compiled;
		compiled.table_rtv = view.renderTarget;
		list->get_graphics().set_rtv(compiled,
			HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0,
			vec4(to_linear(srgb_clear.x), to_linear(srgb_clear.y), to_linear(srgb_clear.z), srgb_clear.w));

		Slots::UI::DisplayOutput output;
		output.GetUi_scale() = 1.0f;
		list->get_graphics().set(output);
	}

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
		const TextureCompare&        compare,
		const std::filesystem::path& reference_dir,
		const std::filesystem::path& results_dir)
	{
		auto actual = readback_texture(tex, compare.sub_resource);
		if (!actual)
			throw TestFailure("check_texture_reference: readback failed for '" + name + "'");

		// TEMP: Vulkan golden-compare investigation -- remove once found.
		if (actual->format == HAL::Format::R16G16B16A16_FLOAT && actual->width > 200)
		{
			auto& m = actual->array[0]->mips[0];
			auto  p = reinterpret_cast<const uint16_t*>(m->data.data() + (size_t)(actual->height / 2) * m->width_stride) + 200 * 4;
			std::ofstream("texture_compare.temp", std::ios::app) << name << " raw fp16 @(200,mid) "
				<< half_to_float(p[0]) << " " << half_to_float(p[1]) << " " << half_to_float(p[2]) << " " << half_to_float(p[3]) << "\n" << std::flush;
		}

		if (actual->format == HAL::Format::R16G16B16A16_FLOAT)
			actual = fp16_to_rgba8(*actual, compare.float_encoding);

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

		// TEMP: Vulkan golden-compare investigation -- remove once found.
		std::ofstream("texture_compare.temp", std::ios::app) << name << " fmt " << (int)actual->format
			<< " act " << actual_rgba->width << "x" << actual_rgba->height << " bytes " << act_data.size() << " fmt " << (int)actual_rgba->format
			<< " ref bytes " << ref_data.size() << " fmt " << (int)reference->format
			<< " bpp " << bpp_act << "/" << bpp_ref << " ch " << ch;
		if (actual_rgba->width > 200)
		{
			size_t p = (size_t)(actual_rgba->height / 2) * actual_rgba->width + 200;
			auto& pre = actual->array[0]->mips[0]->data;
			std::ofstream("texture_compare.temp", std::ios::app) << " | pre-png " << (int)pre[p*4] << "," << (int)pre[p*4+1] << "," << (int)pre[p*4+2]
				<< " act " << (int)(uint8_t)act_data[p*bpp_act] << "," << (int)(uint8_t)act_data[p*bpp_act+1] << "," << (int)(uint8_t)act_data[p*bpp_act+2]
				<< " ref " << (int)(uint8_t)ref_data[p*bpp_ref] << "," << (int)(uint8_t)ref_data[p*bpp_ref+1] << "," << (int)(uint8_t)ref_data[p*bpp_ref+2];
		}
		std::ofstream("texture_compare.temp", std::ios::app) << "\n" << std::flush;

		for (size_t p = 0; p < pixel_count; ++p)
		{
			int max_diff = 0;
			bool pixel_mismatch = false;
			for (size_t c = 0; c < ch; ++c)
			{
				int d = std::abs((int)(uint8_t)act_data[p * bpp_act + c]
				               - (int)(uint8_t)ref_data[p * bpp_ref + c]);
				if (d > max_diff) max_diff = d;
				if (d > (int)compare.tolerance)
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

		const auto allowed_pixels = (size_t)(compare.max_mismatch_fraction * (double)pixel_count);
		if (mismatch_pixels <= allowed_pixels)
		{
			// Within this test's budget: pass, but keep the drift visible.
			Log::get() << Log::LEVEL_WARNING
				<< "[TEXTURE] '" << name << "': " << mismatch_pixels << " / " << pixel_count
				<< " pixels differ (within allowed " << allowed_pixels << ")" << Log::endl;
			return;
		}

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
			+ " pixels differ, allowed " + std::to_string(allowed_pixels)
			+ ", see " + results_dir.string() + "/)");
	}
}
