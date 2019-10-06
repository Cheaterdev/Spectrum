
#include <pch.h>


template <class T>
struct BRDFSignature : public T
{
	using T::T;

	typename T::template Table<0, Render::ShaderVisibility::ALL, Render::DescriptorRange::UAV, 0, 1> target = this;

};



void BRDF::create_new()
{
	Render::ComputePipelineStateDesc desc;
	desc.root_signature = BRDFSignature<SignatureCreator>().create_root();
	desc.shader = Render::compute_shader::get_resource({ "shaders\\BRDF.hlsl", "CS", 0,{} });
	state = desc.create();



	texture.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex3D(DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, 64, 64, 64, 1,  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)));
	Render::CommandList::ptr list(new Render::CommandList(Render::CommandListType::DIRECT));
	list->begin("BRDF");


	Render::ComputeContext& compute_context = list->get_compute();
	compute_context.set_pipeline(state);

	BRDFSignature<Signature> sig(&compute_context);
	sig.target[0]= texture->texture_3d()->get_static_uav();
	list->transition(texture, Render::ResourceState::UNORDERED_ACCESS);
	compute_context.dispach(texture->get_size(), ivec3(4,4,4));
	list->transition(texture, Render::ResourceState::PIXEL_SHADER_RESOURCE);
	list->end();
	list->execute_and_wait();


}


template<class Archive>
void BRDF::serialize(Archive& ar, const unsigned int)
{
	ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(TextureAsset);

}
BOOST_CLASS_EXPORT_IMPLEMENT(BRDF);