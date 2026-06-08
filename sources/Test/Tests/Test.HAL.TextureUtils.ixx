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
		uint                         tolerance     = 0,
		const std::filesystem::path& reference_dir = "test_references",
		const std::filesystem::path& results_dir   = "test_results");
}
