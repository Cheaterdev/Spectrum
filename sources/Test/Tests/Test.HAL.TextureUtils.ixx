export module Test.HAL.TextureUtils;

export import Test.Framework;

import HAL;
import Core;

export namespace Test
{
	HAL::texture_data::ptr readback_texture(HAL::TextureResource* tex, uint sub_resource = 0);

	// Render size for golden-image tests of scenes (meshes, lighting, post):
	// at small sizes a missing triangle or a one-pixel seam falls between
	// pixels. Tests whose size is fixed by the engine (sky cubemaps) or by the
	// test's own pixel geometry (nine-patch clipping) keep theirs.
	constexpr uint kReferenceWidth  = 1920;
	constexpr uint kReferenceHeight = 1080;
	constexpr float kReferenceAspect = float(kReferenceWidth) / float(kReferenceHeight);

	// UI PSOs are built for the scRGB swapchain; check_texture_reference
	// re-encodes this format to sRGB8 before comparing against the goldens.
	inline const HAL::Format UI_RT_FORMAT = HAL::Format::R16G16B16A16_FLOAT;

	// Binds a UI_RT_FORMAT view with a clear, plus the DisplayOutput UI_Render
	// binds on an SDR display. srgb_clear is sRGB-encoded, as it was when these
	// targets were UNORM; it is linearized for the scRGB target.
	void set_ui_target(HAL::CommandList::ptr& list, HAL::Texture2DView& view, vec4 srgb_clear);

	// How a float target (R16G16B16A16_FLOAT, R11G11B10_FLOAT) is turned into
	// the 8-bit values the goldens hold. Other formats are compared as stored.
	enum class FloatEncoding
	{
		Linear,     // scRGB (UI targets): sRGB-encode
		Encoded,    // shader already gamma-encoded (e.g. ColorRTX's pow(1/2.2)): clamp only
		Tonemapped, // HDR (sky, lighting): x exposure, Reinhard per channel, then sRGB-encode
	};

	struct TextureCompare
	{
		// Max per-channel difference (0-255) for a pixel to still count as
		// matching. 2 absorbs PNG encoders rounding exact boundary values (e.g.
		// 0.5*255) differently (DirectXTex on D3D12 vs WIC on Vulkan) and GPUs
		// differing by a unit in the last place.
		uint tolerance = 2;

		// Fraction of pixels (0-1) allowed to exceed `tolerance` before the test
		// fails. 0 = every pixel must match. For output with a little inherent
		// noise, e.g. ray tracing.
		double max_mismatch_fraction = 0.0;

		FloatEncoding float_encoding = FloatEncoding::Linear;
		uint          sub_resource   = 0;

		// FloatEncoding::Tonemapped only.
		float exposure = 1.0f;
	};

	void check_texture_reference(
		HAL::TextureResource*        tex,
		const std::string&           name,
		const TextureCompare&        compare       = {},
		const std::filesystem::path& reference_dir = "test_references",
		const std::filesystem::path& results_dir   = "test_results");

	// Same comparison for data already read back (compare.sub_resource unused),
	// e.g. several subresources fetched in one readback list.
	void check_texture_data_reference(
		HAL::texture_data::ptr       actual,
		const std::string&           name,
		const TextureCompare&        compare       = {},
		const std::filesystem::path& reference_dir = "test_references",
		const std::filesystem::path& results_dir   = "test_results");
}
