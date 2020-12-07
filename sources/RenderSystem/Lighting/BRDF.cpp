
#include <pch.h>


void BRDF::create_new()
{

	texture.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex3D(DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, 64, 64, 64, 1,  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)));
	Render::CommandList::ptr list(new Render::CommandList(Render::CommandListType::DIRECT));
	list->begin("BRDF");


	Render::ComputeContext& compute_context = list->get_compute();

	compute_context.set_pipeline(GetPSO<PSOS::BRDF>());

	{
		Slots::BRDF data;
		data.GetOutput() = texture->texture_3d()->rwTexture3D[0];
		data.set(compute_context);
	}

	compute_context.dispach(texture->get_size(), ivec3(4,4,4));
	list->end();
	list->execute_and_wait();
}


template<class Archive>
void BRDF::serialize(Archive& ar, const unsigned int)
{
	ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(TextureAsset);

}
BOOST_CLASS_EXPORT_IMPLEMENT(BRDF);