module Graphics:BRDF;
import <RenderSystem.h>;

import :TextureAsset;
import :Asset;



REGISTER_TYPE(BRDF);
import HAL;



void BRDF::create_new()
{

	texture.reset(new HAL::Texture(HAL::ResourceDesc::Tex3D(HAL::Format::R16G16B16A16_FLOAT, { 64, 64, 64 }, 1, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess)));
	HAL::CommandList::ptr list(new HAL::CommandList(HAL::CommandListType::DIRECT));
	list->begin("BRDF");


	HAL::ComputeContext& compute_context = list->get_compute();

	compute_context.set_pipeline<PSOS::BRDF>();

	{
		Slots::BRDF data;
		data.GetOutput() = texture->texture_3d().mips[0].rwTexture3D;
		data.set(compute_context);
	}

	compute_context.dispach(texture->get_size(), ivec3(4, 4, 4));
	list->execute_and_wait();
}


template<class Archive>
void BRDF::serialize(Archive& ar, const unsigned int)
{
	SAVE_PARENT(TextureAsset);

}

EngineAsset<BRDF> EngineAssets::brdf(L"brdf", [] {
	BRDF* brdf = new BRDF();
	brdf->create_new();
	return  brdf;
	});