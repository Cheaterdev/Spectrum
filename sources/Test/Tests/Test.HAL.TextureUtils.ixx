export module Test.HAL.TextureUtils;

export import Test.Framework;

import HAL;
import Core;

export namespace Test
{
	HAL::texture_data::ptr readback_texture(HAL::TextureResource* tex, uint sub_resource = 0);

	void check_texture_reference(
		HAL::TextureResource*        tex,
		const std::string&           name,
		uint                         sub_resource  = 0,
		// Allow a 1-LSB difference per channel: golden references and actuals can
		// be encoded by different PNG libraries (DirectXTex on D3D12 vs WIC on
		// Vulkan) which round exact boundary values (e.g. 0.5*255) differently,
		// and GPUs may differ by a unit in the last place.  2 stays well below
		// any perceptible / real rendering error.
		uint                         tolerance     = 2,
		const std::filesystem::path& reference_dir = "test_references",
		const std::filesystem::path& results_dir   = "test_results");
}
