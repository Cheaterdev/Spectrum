module Graphics:BRDF;
import RenderSystem;


import :TextureAsset;
import :Asset;



REGISTER_TYPE(BRDF);
CEREAL_REGISTER_POLYMORPHIC_RELATION(TextureAsset, BRDF);
CEREAL_FORCE_REGISTER(BRDF);
// Asset→TextureAsset is already forced in TextureAsset.cpp; just add the next link.
CEREAL_FORCE_REGISTER_RELATION(TextureAsset, BRDF);
import HAL;



void BRDF::create_new()
{

	texture.reset(new HAL::Texture(RenderSystem::get().device(), HAL::ResourceDesc::Tex3D(HAL::Format::R16G16B16A16_FLOAT, { 64, 64, 64 }, 1, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess)));
	HAL::CommandList::ptr list(new HAL::CommandList(HAL::CommandListType::DIRECT, RenderSystem::get().device()));
	list->begin(L"BRDF");


	HAL::ComputeContext& compute_context = list->get_compute();

	compute_context.set_pipeline<PSOS::BRDF>();

	{
		Slots::BRDF data;
		data.GetOutput() = texture->texture_3d().mips[0].rwTexture3D;
		compute_context.set(data);
	}

	compute_context.dispatch(texture->get_size(), ivec3(4, 4, 4));
	list->execute_and_wait();
}


template<class Archive>
void BRDF::serialize(Archive& ar)
{
	SAVE_PARENT(TextureAsset);

}

EngineAsset<BRDF> EngineAssets::brdf(L"brdf", [] {
	BRDF* brdf = new BRDF();
	brdf->create_new();
	return  brdf;
	});