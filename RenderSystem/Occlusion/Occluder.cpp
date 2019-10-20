#include "pch.h"



template <class T>
struct DebugSignature : public T
{
	using T::T;
	/*
	 root_desc[0] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::VERTEX); //frame data
	root_desc[1] = Render::DescriptorSRV(0, Render::ShaderVisibility::VERTEX);
	*/
	typename T::template ConstBuffer	<0, Render::ShaderVisibility::VERTEX,0>								camera_data = this;
	typename T::template SRV			<1, Render::ShaderVisibility::VERTEX, 0>								nodes = this;
};



void debug_drawer::draw(Render::CommandList& list, camera* cam)
{
    list.get_graphics().set_pipeline(state);
    list.get_graphics().set_topology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    visible_count = 0;
    std::vector<node> nodes;
    size_t count = aabbs.size() ;
    unsigned int index = 0;
    nodes.resize(count);
    int i = 0;
	DebugSignature<Signature> shader_data(&list.get_graphics());

    for (auto& p : aabbs)
    {
        nodes[i].mat = p.second;
        AABB* aabb = static_cast<AABB*>(p.first.get());
        nodes[i].min = aabb->min;
        nodes[i].max = aabb->max;
        nodes[i].color = /*(*cam, *aabb, nodes[i].mat) ? */float4(0, 0, 1, 1)/* : float4(1, 0, 0, 1)*/;
        //   if (aabbs[index]->is_inside(*cam))
//{
        //      visible_count++;
        //	(*const_buffer)[i].color=vec3
        //   }
        index++;
        i++;
    }

	shader_data.camera_data= cam->get_const_buffer();
	shader_data.nodes =  nodes;
    list.get_graphics().draw(24, 0, (UINT)count);
    aabbs.clear();
}

void debug_drawer::add(std::shared_ptr<Primitive> p, mat4x4 m)
{
    if (p->get_type() == primitive_types::aabb)
        aabbs.insert(std::make_pair(p, m));
}

debug_drawer::debug_drawer()
{
    Render::PipelineStateDesc desc;
    desc.rtv.enable_depth = false;
    desc.rtv.ds_format = DXGI_FORMAT_R32_FLOAT;
    //desc.rtv.rtv_formats.e
    desc.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    desc.pixel = Render::pixel_shader::get_resource({ "shaders/debug/box.hlsl", "PS", 0, {} });
    desc.vertex = Render::vertex_shader::get_resource({ "shaders/debug/box.hlsl", "VS", 0, {} });
    desc.rtv.rtv_formats = {DXGI_FORMAT_R16G16B16A16_FLOAT};
    Render::RootSignatureDesc root_desc;
    root_desc[0] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::VERTEX); //frame data
    root_desc[1] = Render::DescriptorSRV(0, Render::ShaderVisibility::VERTEX);
	desc.root_signature = DebugSignature<SignatureCreator>().create_root();// .reset(new Render::RootSignature(root_desc));
    state.reset(new Render::PipelineState(desc));
    /*	const_buffer.reset(new DX11::Buffer<node>(D3D11_USAGE_DEFAULT, D3D11_BIND_CONSTANT_BUFFER, 0, 0, 64));

    for (int i = 0; i < 64; i++)
    {
        (*const_buffer)[i].mat = mat4x4::translation(4 * vec3(i % 8, 0, i / 8));
        (*const_buffer)[i].min = vec3(-1, -1, -1);
        (*const_buffer)[i].max = vec3(1, 1, 1);
    }*/
}


BOOST_CLASS_EXPORT_IMPLEMENT(occluder)

bool occluder::is_inside(const Frustum& f)
{
 //   return true;

   if (!primitive) return true;

    return intersect(f, primitive_global.get());
}

void occluder::debug_draw(debug_drawer& drawer)
{
/*	mat4x4 m;
	m.identity();
     if (primitive_global)
       drawer.add(primitive_global,m);*/
}
