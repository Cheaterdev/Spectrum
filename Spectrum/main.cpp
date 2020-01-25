#include "pch.h"
#include "SSAO.h"
#include <filesystem>

#ifdef OCULUS_SUPPORT
// Include the Oculus SDK
#include "OVR_CAPI_D3D.h"
ovrEyeRenderDesc eyeRenderDesc[2];

#endif


HRESULT device_fail()
{
	auto hr = DX12::Device::get().get_native_device()->GetDeviceRemovedReason();
	Log::get().crash_error(hr);
	return hr;
}


class ShadowMap
{


public:
	Render::Texture::ptr depth_tex;
	RenderTargetTable table;


	camera light_cam;

	void resize(ivec2 size)
	{
		depth_tex.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, size.x, size.y, 0, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL), Render::ResourceState::PIXEL_SHADER_RESOURCE));
		table = RenderTargetTable({}, depth_tex);
		light_cam.set_projection_params(pi / 4.0f, 1.0f, 0.1f, 100.f);
		light_cam.position = { 0, 40, 1 };
		light_cam.target = { 0, 0, 0 };
		light_cam.update();
	}

	void set(MeshRenderContext::ptr& context)
	{
		auto& list = context->list;
		list->transition(depth_tex, Render::ResourceState::DEPTH_WRITE);
		//depth_tex->change_state(list, Render::ResourceState::PIXEL_SHADER_RESOURCE, Render::ResourceState::DEPTH_WRITE);
		table.clear_depth(context);
		table.set(context);
		list->get_graphics().set_viewport(depth_tex->texture_2d()->get_viewport());
		list->get_graphics().set_scissor(depth_tex->texture_2d()->get_scissor());
		context->cam = &light_cam;
		context->override_material = nullptr;
	}

	void end(MeshRenderContext::ptr& context)
	{
		//	auto& list = context->list;
			//depth_tex->change_state(list, Render::ResourceState::DEPTH_WRITE, Render::ResourceState::PIXEL_SHADER_RESOURCE);
	}



};
/*
class DeferredShading
{
	Render::ComputePipelineState::ptr state;
public:

	DeferredShading()
	{
		Render::RootSignatureDesc root_desc;
		root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 4);
		root_desc[1] = Render::DescriptorTable(Render::DescriptorRange::UAV, Render::ShaderVisibility::ALL, 0, 1);
		root_desc[2] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::ALL);
		root_desc[3] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 4, 1);
		root_desc[4] = Render::DescriptorConstBuffer(1, Render::ShaderVisibility::ALL);
		Render::ComputePipelineStateDesc desc;
		desc.shader = Render::compute_shader::get_resource({ "shaders\\DeferredShading.hlsl", "CS", 0, {} });
		desc.root_signature.reset(new Render::RootSignature(root_desc));
		state.reset(new  Render::ComputePipelineState(desc));
	}


	void shade(MeshRenderContext::ptr& context, G_Buffer& buffer, ShadowMap& map)
	{
		context->list->get_compute().set_pipeline(state);
		context->list->get_compute().set(0, buffer.srv_table);
		//      context->list->get_compute().set_table(1, buffer.light_tex->texture_2d()->get_uav());
		context->list->get_compute().set_const_buffer(2, context->cam->get_const_buffer());
		context->list->get_compute().set_table(3, map.depth_tex->texture_2d()->get_srv());
		context->list->get_compute().set_const_buffer(4, map.light_cam.get_const_buffer());
		//     ivec2 sizes = { buffer.light_tex->get_desc().Width , buffer.light_tex->get_desc().Height };
		//    context->list->get_compute().dispach(sizes);
	}
};
*/
class PostProcessGraph;
class PostProcessContext;

using ContextPostProcess = FlowGraph::ContextOptions<PostProcessContext>;


class PostProcessContext :public FlowGraph::GraphContext
{
	std::list<std::future<void>> tasks;
protected:
	virtual void add_task(FlowGraph::Node* node) override
	{
		node->on_start(this);
		run(node);


		node->on_done(this);
	}
public:
	G_Buffer* g_buffer;
	MeshRenderContext::ptr mesh_context;
	PSSM* pssm;
	Scene::ptr& scene;
	main_renderer::ptr renderer;
	bool realtime_debug = false;
	PostProcessContext(Scene::ptr& scene) :scene(scene)
	{

	}

	void wait()
	{
		for (auto& t : tasks)
			t.wait();
	}
};


class EditorNode :public ContextPostProcess::NodeType, public GUI::Elements::FlowGraph::VisualGraph
{

	Render::Texture::ptr debug_texture;

	std::unique_ptr<Timer> timer;
protected:




	virtual FlowGraph::input::ptr register_input(FlowGraph::data_types type, std::string name)override
	{
		auto res = ContextPostProcess::NodeType::register_input(type, name);

		return res;
	}

	virtual FlowGraph::output::ptr register_output(FlowGraph::data_types type, std::string name) override
	{
		auto res = ContextPostProcess::NodeType::register_output(type, name);

		res->immediate_send_next = false;
		return res;
	}

	virtual void on_start(PostProcessContext* c) override
	{
		timer = std::make_unique<Timer>(std::move(c->mesh_context->list->start(convert(name).c_str())));

	}
	virtual void on_done(PostProcessContext* c) override
	{

		if (c->realtime_debug && debug_texture)
		{
			MipMapGenerator::get().copy_texture_2d_slow(c->mesh_context->list->get_graphics(), img_inner->texture.texture, c->g_buffer->result_tex.first());
			c->mesh_context->list->transition(img_inner->texture.texture, ResourceState::PIXEL_SHADER_RESOURCE);
		}
		timer.reset();
		ContextPostProcess::NodeType::on_done(c);
	}

	GUI::Elements::image::ptr img_inner;

	GUI::base::ptr create_editor_window() override
	{

		if (!debug_texture)
			debug_texture.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, 128, 128, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET), Render::ResourceState::PIXEL_SHADER_RESOURCE));
		GUI::Elements::image::ptr img(new GUI::Elements::image);
		img->texture.texture = Render::Texture::get_resource({ "textures/gui/shadow.png", false, false });
		img->texture.padding = { 9, 9, 9, 9 };
		img->padding = { 9, 9, 9, 9 };
		img->width_size = GUI::size_type::MATCH_CHILDREN;
		img->height_size = GUI::size_type::MATCH_CHILDREN;
		//   img->size = { 64, 64 };
		img_inner.reset(new GUI::Elements::image);


		img_inner->texture.texture = debug_texture;
		img_inner->docking = GUI::dock::TOP;
		img_inner->size = { 128, 128 };
		img->add_child(img_inner);
		return img;
	}


};
/*
template<class... Args>
class Params
{
	using type = Args...;
};
*/

template <class ... Args> struct Inputs
{
	using type = std::tuple<Args...>;
};
template <class ... Args> struct Outputs
{
	using type = std::tuple<Args...>;
};


template <class O, class ...I> class TemplatedNode : public EditorNode
{


	template<int i>
	void register_i_type()
	{

	}


	template<int i, class P>
	void register_i_type(P t)
	{
		//		std::get<i>(t);

		register_input(FlowGraph::data_types::INT, "life");
	}


	template<int i, class P, class ...A>
	void register_i_type(P o, A...a)
	{
		register_i_type<i>(o);
		register_i_type<i + 1>(a...);


	}



	template<int i>
	void register_o_type()
	{

	}


	template<int i, class P>
	void register_o_type(P t)
	{
		//	std::get<i>(t);

		register_output(FlowGraph::data_types::INT, "life");
	}


	template<int i, class P, class ...A>
	void register_o_type(P o, A...a)
	{
		register_o_type<i>(o);
		register_o_type<i + 1>(a...);
	}


protected:
	using result_type = typename O::type;
	using input_type = std::tuple<I...>;

	virtual result_type make_result(I...) = 0;

	TemplatedNode()
	{
		{
			input_type t;
			register_i_type<0>(t);
		}

		{
			typename O::type t;
			register_o_type<0>(t);
		}
	}

};

class TestNode :public TemplatedNode< Inputs<int>, int >
{
public:
	using ptr = std::shared_ptr<TestNode>;
	void operator()(PostProcessContext* context)
	{
	}

	virtual result_type make_result(int i)
	{

		return { i };
	}


	TestNode()
	{
		//register_input(FlowGraph::data_types::INT, "life");
		//register_output(FlowGraph::data_types::INT, "life");
		//name = "PostProcess";
		auto res = make_result({ 2 });
	}
} *node;



class PostProcessGraph :public FlowGraph::graph
{
public:
	using ptr = std::shared_ptr<PostProcessGraph>;
	PostProcessGraph()
	{
		register_input(FlowGraph::data_types::INT, "life");
		register_output(FlowGraph::data_types::INT, "life");
		name = "PostProcess";
	}
};



class AdaptationNode :public EditorNode
{
	HDRAdaptation::ptr adaptation;

	void operator()(PostProcessContext* context)
	{
		if (!adaptation) adaptation = std::make_shared<HDRAdaptation>(*context->g_buffer);
		adaptation->process(context->mesh_context);
		get_output(0)->put(0);
	}
public:
	AdaptationNode()
	{
		register_input(FlowGraph::data_types::INT, "life");
		register_output(FlowGraph::data_types::INT, "life");
		name = "Adaptation";
	}
};


class SkyNode :public EditorNode
{
	SkyRender::ptr sky;
	std::mutex m;
	void operator()(PostProcessContext* context)
	{
		if (!sky)
		{
			m.lock();
			if (!sky) sky = std::make_shared<SkyRender>();
			m.unlock();
		}
		get_output(0)->put(sky);
	}
public:
	SkyNode()
	{
		register_output(FlowGraph::data_types("sky"), "sky");
		name = "Sky";
	}
};


class SkyRenderNode :public EditorNode
{
	void operator()(PostProcessContext* context)
	{
		auto sky = get_input(1)->get<SkyRender::ptr>();
		sky->process(context->mesh_context);
		get_output(0)->put(0);
	}





public:
	SkyRenderNode()
	{
		register_input(FlowGraph::data_types::INT, "life");
		register_input(FlowGraph::data_types("sky"), "sky");
		register_output(FlowGraph::data_types::INT, "life");
		name = "SkyRender";
	}
};

class SkyEnviromentNode :public EditorNode
{
	void operator()(PostProcessContext* context)
	{
		auto sky = get_input(0)->get<SkyRender::ptr>();
		sky->update_cubemap(context->mesh_context);
		get_output(0)->put(sky->enviroment);
	}
public:
	SkyEnviromentNode()
	{

		register_input(FlowGraph::data_types("sky"), "sky");
		register_output(FlowGraph::data_types("cubemap"), "enviroment");
		name = "SkyEnv";
	}
};

class SMAANode :public EditorNode
{
	SMAA::ptr smaa;

	void operator()(PostProcessContext* context)
	{
		if (!smaa) smaa = std::make_shared<SMAA>(*context->g_buffer);
		smaa->process(context->mesh_context);
		get_output(0)->put(0);
	}
public:
	SMAANode()
	{
		register_input(FlowGraph::data_types::INT, "life");
		register_output(FlowGraph::data_types::INT, "life");
		name = "SMAA";
	}
};


class VoxelNode :public EditorNode
{
	bool enabled = true;
	//bool ssgi_enabled = true;

	VoxelGI::ptr gi;

	VoxelGI::VISUALIZE_TYPE viz_type = VoxelGI::VISUALIZE_TYPE::FULL;
	FlowGraph::input::ptr enviroment;
	void operator()(PostProcessContext* context)
	{
		if (!gi) gi = std::make_shared<VoxelGI>(context->scene, *context->g_buffer);

		//	if (ssgi_enabled)
		//		gi->ssgi_tex = get_input(2)->get<Render::Texture::ptr>();
		//	else
		//		gi->ssgi_tex = Render::Texture::null;

		if (enabled) {
			auto buf = context->mesh_context->g_buffer;
			context->mesh_context->g_buffer = nullptr;
			gi->render_type = viz_type;
			gi->generate(context->mesh_context, context->renderer, *context->pssm, enviroment->get<Enviroment>());
			context->mesh_context->g_buffer = buf;
		}
		get_output(0)->put(0);

	}

	GUI::base::ptr create_editor_window() override
	{
		auto res = EditorNode::create_editor_window();

		{
			auto check = std::make_shared<GUI::Elements::check_box>();

			check->docking = GUI::dock::TOP;
			check->on_check = [this](bool v) {
				enabled = v;
			};

			check->set_checked(enabled);

			res->add_child(check);

		}
		/*{
			auto check = std::make_shared<GUI::Elements::check_box>();

			check->docking = GUI::dock::TOP;
			check->on_check = [this](bool v) {
				ssgi_enabled = v;
			};

			check->set_checked(enabled);

			res->add_child(check);
		}*/

		auto type_selector = std::make_shared<GUI::Elements::combo_box>();
		type_selector->add_item("Full")->on_select = [this]() {viz_type = VoxelGI::VISUALIZE_TYPE::FULL; };
		type_selector->add_item("Reflection")->on_select = [this]() {viz_type = VoxelGI::VISUALIZE_TYPE::REFLECTION; };
		type_selector->add_item("Indirect")->on_select = [this]() {viz_type = VoxelGI::VISUALIZE_TYPE::INDIRECT; };
		type_selector->add_item("Voxels")->on_select = [this]() {viz_type = VoxelGI::VISUALIZE_TYPE::VOXEL; };
		type_selector->add_item("Voxels Albedo")->on_select = [this]() {viz_type = VoxelGI::VISUALIZE_TYPE::FULL; };
		type_selector->add_item("Voxels Normal")->on_select = [this]() {viz_type = VoxelGI::VISUALIZE_TYPE::FULL; };


		type_selector->docking = GUI::dock::TOP;
		res->add_child(type_selector);

		return res;
	}
public:
	VoxelNode()
	{
		register_input(FlowGraph::data_types::INT, "life");
		register_output(FlowGraph::data_types::INT, "life");

		enviroment = register_input(FlowGraph::data_types("cubemap"), "enviroment");
		//	register_input(FlowGraph::data_types("ssgi"), "ssgi")->default_value = Render::Texture::null;

		name = "VoxelGI";
	}
};


/*
class SSGINode :public EditorNode
{
	bool enabled = true;

	SSGI::ptr ssgi;

	void operator()(PostProcessContext* context)
	{
		if (!ssgi) ssgi = std::make_shared<SSGI>( *context->g_buffer);


		ssgi->process(context->mesh_context, *context->g_buffer);
		get_output(1)->put(ssgi->reflect_tex);
		get_output(0)->put(0);
	}

public:
	SSGINode()
	{
		register_input(FlowGraph::data_types::INT, "life");
		register_output(FlowGraph::data_types::INT, "life");
		register_output(FlowGraph::data_types("ssgi"), "ssgi");

		name = "SSGI";
	}
};
*/



/*
class SkyRender
{
		Render::ComputePipelineState::ptr state;
		Render::Texture::ptr transmittance;
		Render::Texture::ptr irradiance;
		Render::Texture::ptr inscatter;
		Render::HandleTable table;


	public:

		SkyRender()
		{
			Render::RootSignatureDesc root_desc;
			root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 4);
			root_desc[1] = Render::DescriptorTable(Render::DescriptorRange::UAV, Render::ShaderVisibility::ALL, 0, 1);
			root_desc[2] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::ALL);
			root_desc[3] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 4, 3);
			root_desc[4] = Render::DescriptorTable(Render::DescriptorRange::SAMPLER, Render::ShaderVisibility::ALL, 0, 1);
			Render::ComputePipelineStateDesc desc;
			desc.shader = Render::compute_shader::get_resource({ "shaders\\sky.hlsl", "CS", 0, {} });
			desc.root_signature.reset(new Render::RootSignature(root_desc));
			state.reset(new  Render::ComputePipelineState(desc));
			transmittance = Render::Texture::get_resource({"textures\\Transmit.dds", false, false});
			irradiance = Render::Texture::get_resource({ "textures\\irradianceTexture.dds", false, false });
			inscatter = Render::Texture::get_resource({ "textures\\inscatterTexture.dds", false, false });
			table = Render::DescriptorHeapManager::get().get_csu()->create_table(3);
			transmittance->place_srv(table[0]);
			irradiance->place_srv(table[1]);
			inscatter->place_srv(table[2]);
		}


		void process(MeshRenderContext::ptr& context, G_Buffer& buffer)
		{
			context->list->get_compute().set_pipeline(state);
			context->list->get_compute().set(0, buffer.srv_table);
			context->list->get_compute().set(1, buffer.light_tex->get_uav_handle());
			context->list->get_compute().set_const_buffer(2, context->cam->get_const_buffer());
			context->list->get_compute().set(3, table);
			context->list->get_compute().set(4, Render::DescriptorHeapManager::get().get_default_samplers());
			ivec2 sizes = { buffer.light_tex->get_desc().Width , buffer.light_tex->get_desc().Height };
			context->list->get_compute().dispach(sizes, {16, 16});
		}



};


*/



class GBufferDownsamplerNode :public EditorNode
{
	//GBufferDownsampler::ptr downsampler;
	void operator()(PostProcessContext* context)
	{
		//	if (!downsampler) downsampler = std::make_shared<GBufferDownsampler>(*context->g_buffer);
		//	downsampler->process(context->mesh_context);
		for (auto& eye : context->mesh_context->eye_context->eyes)
			MipMapGenerator::get().generate_quality(context->mesh_context->list->get_graphics(), eye.cam, *eye.g_buffer);

		get_output(0)->put(0);
	}
public:
	GBufferDownsamplerNode()
	{
		register_input(FlowGraph::data_types::INT, "life");
		register_output(FlowGraph::data_types::INT, "life");
		name = "Quality";
	}
};


class LightingNode :public EditorNode
{

	G_Buffer* buff = nullptr;
	void operator()(PostProcessContext* context)
	{
		if (buff != context->g_buffer)
		{
			context->g_buffer->size.register_change(&lighting, [this](ivec2 size) {
				lighting.pssm.resize(size);
				});
			buff = context->g_buffer;
		}
		//if (!downsampler) downsampler = std::make_shared<GBufferDownsampler>(*context->g_buffer);
	//	downsampler->process(context->mesh_context);
		auto cam = context->mesh_context->cam;
		lighting.process(context->mesh_context, *context->g_buffer, context->renderer, context->scene);
		context->mesh_context->cam = cam;
		context->pssm = &lighting.pssm;

		get_output(0)->put(0);
	}
public:

	LightSystem lighting;

	LightingNode()
	{
		register_input(FlowGraph::data_types::INT, "life");
		register_output(FlowGraph::data_types::INT, "life");
		name = "Lighting";
	}
};

/*

class tiled_image : public GUI::base
{
	TiledTexture::ptr tiles;

	Render::PipelineState::ptr state;
public:
	using ptr = s_ptr<tiled_image>;

	tiled_image()
	{
		size = { 16384, 16384 };
		tiles.reset(new TiledTexture("texture.jpg"));
		Render::PipelineStateDesc state_desc;
		Render::RootSignatureDesc root_desc;
		root_desc[0] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::VERTEX);
		root_desc[1] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::PIXEL);
		root_desc[2] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::PIXEL, 0, 5);
		root_desc[3] = Render::DescriptorTable(Render::DescriptorRange::SAMPLER, Render::ShaderVisibility::PIXEL, 0, 3);
		root_desc[4] = Render::DescriptorTable(Render::DescriptorRange::UAV, Render::ShaderVisibility::PIXEL, 0, 1);
		state_desc.root_signature.reset(new Render::RootSignature(root_desc));
		state_desc.pixel = Render::pixel_shader::get_resource({ "shaders\\tiles.hlsl", "PS", 0, {} });
		state_desc.vertex = Render::vertex_shader::get_resource({ "shaders\\tiles.hlsl", "VS", 0, {} });
		state.reset(new Render::PipelineState(state_desc));
	}


	virtual void draw(Render::context& t) override
	{
		tiles->update(t.command_list->get_ptr());
		auto& list = t.command_list->get_graphics();
		list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		list.set_pipeline(state);
		auto bounds = get_render_bounds();
		float2 p1 = (vec2(bounds.pos)) / get_user_ui()->size.get();
		float2 p2 = (vec2(bounds.pos) + vec2(bounds.size)) / get_user_ui()->size.get();
		list.set_const_buffer_raw(0, sizer(p1, p2));
		list.set(2, tiles->table);
		list.set(3, tiles->sampler_table);
		list.set(4, tiles->visibility_texture->get_uav());
		list.draw(4);
	}
};*/
/*
class CubeMapDrawer
{

		Render::PipelineState::ptr state;
		Render::Texture::ptr transmittance;
		Render::Texture::ptr irradiance;
		Render::Texture::ptr inscatter;
		Render::HandleTable table;
		RenderTargetTable render_table;


	public:

		CubeMapDrawer()
		{
			Render::PipelineStateDesc state_desc;
			Render::RootSignatureDesc root_desc;
			root_desc[0] = Render::DescriptorConstants(0, 1, Render::ShaderVisibility::ALL);
			root_desc[1] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 3);
			root_desc[2] = Render::DescriptorTable(Render::DescriptorRange::SAMPLER, Render::ShaderVisibility::ALL, 0, 2);
			state_desc.root_signature.reset(new Render::RootSignature(root_desc));
			state_desc.pixel = Render::pixel_shader::get_resource({ "shaders\\cubemap.hlsl", "PS", 0, {} });
			state_desc.vertex = Render::vertex_shader::get_resource({ "shaders\\cubemap.hlsl", "VS", 0, {} });
			state_desc.rtv.rtv_formats.resize(1);
			state_desc.rtv.rtv_formats[0] = DXGI_FORMAT_R11G11B10_FLOAT;
			state.reset(new Render::PipelineState(state_desc));
			transmittance = Render::Texture::get_resource({ "textures\\Transmit.dds", false, false });
			irradiance = Render::Texture::get_resource({ "textures\\irradianceTexture.dds", false, false });
			inscatter = Render::Texture::get_resource({ "textures\\inscatterTexture.dds", false, false });
			table = Render::DescriptorHeapManager::get().get_csu()->create_table(3);
			transmittance->texture_2d()->place_srv(table[0]);
			irradiance->texture_2d()->place_srv(table[1]);
			inscatter->texture_3d()->place_srv(table[2]);
		}

		void process(MeshRenderContext::ptr& context, Render::Texture::ptr cubemap)
		{
			auto& list = context->list->get_graphics();
			list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			list.set_pipeline(state);
			std::vector<Render::Viewport> vps;
			std::vector<sizer_long> scissors;
			vps.resize(1);
			vps[0].MinDepth = 0.0f;
			vps[0].MaxDepth = 1.0f;
			scissors.resize(1);
			vps[0].Width = cubemap->get_desc().Width;
			vps[0].Height = cubemap->get_desc().Height;
			vps[0].TopLeftX = 0;
			vps[0].TopLeftY = 0;
			scissors[0] = { 0, 0,  cubemap->get_desc().Width, cubemap->get_desc().Height };
			context->list->get_graphics().set_viewports(vps);
			context->list->get_graphics().set_scissors(scissors[0]);
			context->list->get_graphics().set(1, table);
			context->list->get_graphics().set(2, Render::DescriptorHeapManager::get().get_default_samplers());

			for (unsigned int i = 0; i < 6; i++)
			{
				context->list->get_graphics().set_rtv(1, cubemap->cubemap()->get_rtv(i, 0), Render::Handle());
				context->list->get_graphics().set_constants(0, i);
				list.draw(4);
			}
		}
};
*/


template <class T>
struct ViewportSignatureDesc : public T
{
	using T::T;

	typename T::template Table			<0, Render::ShaderVisibility::PIXEL, Render::DescriptorRange::SRV, 0, 8> input_array = this;
	typename T::template Constants		<1, Render::ShaderVisibility::PIXEL, 1, 1>								 constants = this;
	typename T::template Table			<2, Render::ShaderVisibility::ALL, Render::DescriptorRange::UAV, 0, 1, 1>motion = this;

	typename T::template Sampler<0, Render::ShaderVisibility::ALL, 0> linear{ Render::Samplers::SamplerLinearWrapDesc, this };
	typename T::template Sampler<1, Render::ShaderVisibility::ALL, 0> point{ Render::Samplers::SamplerPointClampDesc, this };

};

using ViewportSignature = SignatureTypes <ViewportSignatureDesc>;



class ViewPortRenderer
{
	//	Render::PipelineState::ptr state;
	ViewportSignature::RootSignature::ptr root_sig;
	Cache<DXGI_FORMAT, ViewportSignature::Pipeline::ptr > states;
	float time = 0;
public:




	ViewPortRenderer()
	{

		root_sig = ViewportSignature::create_typed_root();

		states.create_func = [this](const DXGI_FORMAT& format) ->ViewportSignature::Pipeline::ptr {

			Render::PipelineStateDesc desc;
			desc.root_signature = root_sig;
			desc.rtv.rtv_formats = { format };
			desc.blend.render_target[0].enabled = false;
			desc.vertex = Render::vertex_shader::get_resource({ "shaders\\ViewPortRender.hlsl", "VS", 0,{} });
			desc.pixel = Render::pixel_shader::get_resource({ "shaders\\ViewPortRender.hlsl", "PS", 0,{} });
			return std::make_shared<ViewportSignature::Pipeline>(desc);
		};



	}

	void process(MeshRenderContext::ptr& context, TemporalAA& temporal, Render::Texture::ptr target)
	{
		auto timer = context->list->start(L"last renderer");
		time += context->delta_time;
		auto& list = *context->list;
		auto& graphics = context->list->get_graphics();
		//	buffer->result_tex.swap(context->list, Render::ResourceState::RENDER_TARGET, Render::ResourceState::PIXEL_SHADER_RESOURCE);

		list.transition(target, Render::ResourceState::RENDER_TARGET);
		list.transition(context->g_buffer->albedo_tex, Render::ResourceState::PIXEL_SHADER_RESOURCE);
		graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		auto& shader_data = graphics.set_pipeline_typed(states[target->get_desc().Format]);
		//ViewportSignature<Signature> shader_data(&graphics);



	//	auto& tex = temporal.get_current();
		graphics.set_viewport(target->texture_2d()->get_viewport(0));
		graphics.set_scissor(target->texture_2d()->get_scissor(0));
		graphics.set_rtv(1, target->texture_2d()->get_rtv(0), Render::Handle());
		//temporal.set(context->list, 0);

		shader_data.motion[0] = context->g_buffer->speed_tex->texture_2d()->get_static_srv();
		shader_data.input_array[0] = context->g_buffer->result_tex.first()->texture_2d()->get_static_srv();


		//	graphics.set_dynamic(0, 0,);
			//list.set_dynamic(0, 1, context->g_buffer->albedo_tex->texture_2d()->get_static_srv());
			//list.set_dynamic(0, 2, context->g_buffer->albedo_tex->texture_2d()->get_static_srv());
			//	list.set_const_buffer(1, const_buffer);
			//list.set(1, Render::DescriptorHeapManager::get().get_default_samplers());
		shader_data.constants.set(1, time);
		graphics.draw(4);
	}


	void process(MeshRenderContext::ptr& context)
	{
		auto timer = context->list->start(L"last renderer");
		time += context->delta_time;
		auto& list = *context->list;
		auto& graphics = context->list->get_graphics();

		graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		auto& shader_data = graphics.set_signature_typed<ViewportSignature>(root_sig);
		//	ViewportSignature<Signature> shader_data(&graphics);

		shader_data.constants.set(time);


		for (auto& e : context->eye_context->eyes)
		{
			auto target = e.color_buffer;
			auto g_buffer = e.g_buffer;

			list.transition(target, Render::ResourceState::RENDER_TARGET); //////////////////////////////////////////////////////////////////////////////////////////////////
			list.transition(g_buffer->albedo_tex, Render::ResourceState::PIXEL_SHADER_RESOURCE);
		}

		for (auto& e : context->eye_context->eyes)
		{
			auto target = e.color_buffer;
			auto g_buffer = e.g_buffer;
			graphics.set_pipeline_typed(states[target->get_desc().Format]);
			graphics.set_viewport(target->texture_2d()->get_viewport(0));
			graphics.set_scissor(target->texture_2d()->get_scissor(0));
			graphics.set_rtv(1, target->texture_2d()->get_rtv(0), Render::Handle());


			shader_data.motion[0] = g_buffer->speed_tex->texture_2d()->get_static_srv();
			shader_data.input_array[0] = g_buffer->result_tex.second()->texture_2d()->get_static_srv();

			graphics.draw(4);
		}


	}





};


template <class T>
struct RaytracingDesc : public T
{
	using T::T;

	typename T::template Table			<0, Render::ShaderVisibility::ALL, Render::DescriptorRange::UAV, 0, 1> target = this;
	typename T::template SRV			<1, Render::ShaderVisibility::ALL, 0> scene = this;
	typename T::template ConstBuffer	<2, Render::ShaderVisibility::ALL, 0> camera = this;
	typename T::template Table			<3, Render::ShaderVisibility::ALL, Render::DescriptorRange::SRV, 0, 4, 1> g_buffer = this;
	typename T::template  SRV			<4, Render::ShaderVisibility::ALL, 1> instance_data = this;


	typename T::template Table			<5, Render::ShaderVisibility::ALL, Render::DescriptorRange::SRV, 0, 4, 2> vertex_buffers = this;
	typename T::template Table			<6, Render::ShaderVisibility::ALL, Render::DescriptorRange::SRV, 0, 4, 3> index_buffers = this;
	


	typename T::template Sampler<0, Render::ShaderVisibility::ALL, 0> linear{ Render::Samplers::SamplerLinearWrapDesc, this };
	typename T::template Sampler<1, Render::ShaderVisibility::ALL, 0> point{ Render::Samplers::SamplerPointClampDesc, this };
	typename T::template Sampler<2, Render::ShaderVisibility::ALL, 0> aniso{ Render::Samplers::SamplerAnisoWrapDesc, this };
};

using RaytracingSignature = SignatureTypes <RaytracingDesc>;





const wchar_t* c_hitGroupName = L"MyHitGroup";
const wchar_t* c_raygenShaderName = L"MyRaygenShader";
const wchar_t* c_closestHitShaderName = L"MyClosestHitShader";
const wchar_t* c_missShaderName = L"MyMissShader";




struct RayGenConstantBuffer
{
	UINT texture_offset;
	UINT node_offset;
};

template <class T>
struct RaytracingLocalDesc : public T
{
	using T::T;
	typename T::template ConstBuffer	<0, Render::ShaderVisibility::ALL, 1>				mat_const = this;
	typename T::template Table			<1, Render::ShaderVisibility::ALL, Render::DescriptorRange::SRV, 0, 1, 4> material_textures = this;

};

using RaytracingLocalSignature = SignatureTypes <RaytracingLocalDesc>;



using shader_identifier = std::array<std::byte, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES>;
shader_identifier identify(void* data)
{
	shader_identifier result;

	memcpy(result.data(), data, result.size());

	return result;
}
#define CACHE_ALIGN(x) __declspec(align(x))

CACHE_ALIGN(64)
struct closesthit_identifier
{
	shader_identifier identifier;

	D3D12_GPU_VIRTUAL_ADDRESS mat_buffer;
	D3D12_GPU_DESCRIPTOR_HANDLE gpu;

	//	RayGenConstantBuffer cb;
};

struct RayTracingShaders
{
	Resource::ptr m_missShaderTable;
	Resource::ptr m_hitGroupShaderTable;
	Resource::ptr m_rayGenShaderTable;

	shader_identifier rayGenShaderIdentifier;
	shader_identifier missShaderIdentifier;
	shader_identifier missShadowShaderIdentifier;

	std::vector<closesthit_identifier> hitGroupShaderIdentifiers;

	ComPtr<ID3D12StateObject> m_dxrStateObject;


	RaytracingSignature::RootSignature::ptr signature;
	RaytracingLocalSignature::RootSignature::ptr signature_local;

	std::string blob;

	RayGenConstantBuffer m_rayGenCB;


	ArraysHolder<Render::Handle> buffersVertex;
	ArraysHolder<Render::Handle> buffersIndex;

	//std::vector<ComPtr<IDxcBlob>> blobs;

	struct InstanceData
	{
		UINT vertexBuffer;
		UINT indexBuffer;
		//UINT materialOffset;
	};
	using ptr = std::shared_ptr<RayTracingShaders>;

	ArraysHolder<InstanceData> instanceData;
	std::vector<materials::universal_material::ptr> materials;

//	Render::HandleTable gpu_textures;

	//ArraysHolder<Render::Handle> textures;
	//std::map<materials::universal_material*, size_t> material_offsets;
	std::map<materials::universal_material*, Render::HandleTable> material_textures;

	// Create a raytracing pipeline state object (RTPSO).
	// An RTPSO represents a full set of shaders reachable by a DispatchRays() call,
	// with all configuration options resolved, such as local signatures and other state.
	void CreateRaytracingPipelineStateObject()
	{
	//	materials.resize(1);
		// Create 7 subobjects that combine into a RTPSO:
		// Subobjects need to be associated with DXIL exports (i.e. shaders) either by way of default or explicit associations.
		// Default association applies to every exported shader entrypoint that doesn't have any of the same type of subobject associated with it.
		// This simple sample utilizes default shader association except for local root signature subobject
		// which has an explicit association specified purely for demonstration purposes.
		// 1 - DXIL library
		// 1 - Triangle hit group
		// 1 - Shader config
		// 2 - Local root signature and association
		// 1 - Global root signature
		// 1 - Pipeline config
		CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

		// Local root signature to be used in a ray gen shader.

		auto localRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
		localRootSignature->SetRootSignature(signature_local->get_native().Get());

		// Shader association
		auto rootSignatureAssociation = raytracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);


		// DXIL library
		// This contains the shaders and their entrypoints for the state object.
		// Since shaders are not considered a subobject, they need to be passed in via DXIL library subobjects.

		{
			auto lib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
			D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void*)blob.data(), blob.size());
			lib->SetDXILLibrary(&libdxil);
			// Define which shader exports to surface from the library.
			// If no shader exports are defined for a DXIL library subobject, all shaders will be surfaced.
			// In this sample, this could be omitted for convenience since the sample uses all shaders in the library. 
			{
				lib->DefineExport(c_raygenShaderName);
				lib->DefineExport(c_missShaderName);

				lib->DefineExport(L"ShadowMissShader");
				lib->DefineExport(L"ShadowClosestHitShader");

			}
		}

		{
			auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
			hitGroup->SetClosestHitShaderImport(L"ShadowClosestHitShader");
			hitGroup->SetHitGroupExport(L"ShadowClosestHitGroup");
			hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

		//	rootSignatureAssociation->AddExport(L"ShadowClosestHitGroup");
		}





			/*


		{
			auto lib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
			D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void*)blob->GetBufferPointer(), blob->GetBufferSize());
			lib->SetDXILLibrary(&libdxil);
			// Define which shader exports to surface from the library.
			// If no shader exports are defined for a DXIL library subobject, all shaders will be surfaced.
			// In this sample, this could be omitted for convenience since the sample uses all shaders in the library.
			{
			//	lib->DefineExport(c_raygenShaderName);
			//	lib->DefineExport(c_missShaderName);
				lib->DefineExport(c_closestHitShaderName);
			}
		}
		*/

	
		
		for (auto& mat : materials)
		{

		
			/*auto handles = mat->get_texture_handles();
			auto it = material_offsets.find(mat.get());
			
		/*	if (it != material_offsets.end())
			{
				if (it->second != -1)
					textures.release(it->second);
			}*/
			/*if (handles.size())
			{
				const auto index = material_offsets[mat.get()] = textures.get_free(handles.size());

				for (auto i = 0; i < handles.size(); i++)
				{
					textures[index + i] = handles[i];
				}
			}
			*/

			auto& blob = mat->raytracing_blob;
			// DXIL library
			// This contains the shaders and their entrypoints for the state object.
			// Since shaders are not considered a subobject, they need to be passed in via DXIL library subobjects.
			auto lib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
			D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void*)blob.data(), blob.size());
			lib->SetDXILLibrary(&libdxil);
			// Define which shader exports to surface from the library.
			// If no shader exports are defined for a DXIL library subobject, all shaders will be surfaced.
			// In this sample, this could be omitted for convenience since the sample uses all shaders in the library. 
			{
				lib->DefineExport(mat->wshader_name.c_str(),c_closestHitShaderName);
				
			}

			{
				auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
				hitGroup->SetClosestHitShaderImport(mat->wshader_name.c_str());
				hitGroup->SetHitGroupExport((mat->wshader_name+L"HIT").c_str());
				hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

				rootSignatureAssociation->AddExport((mat->wshader_name + L"HIT").c_str());

			}

		//	break;//
		}



		// Shader config
		// Defines the maximum sizes in bytes for the ray payload and attribute structure.
		auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
		UINT payloadSize = 6 * sizeof(float);   // float4 color
		UINT attributeSize = 2 * sizeof(float); // float2 barycentrics
		shaderConfig->Config(payloadSize, attributeSize);

		// Local root signature and shader association


		// This is a root signature that enables a shader to have unique arguments that come from shader tables.

		// Global root signature
		// This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
		auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
		globalRootSignature->SetRootSignature(signature->get_native().Get());

		// Pipeline config
		// Defines the maximum TraceRay() recursion depth.
		auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
		// PERFOMANCE TIP: Set max recursion depth as low as needed 
		// as drivers may apply optimization strategies for low recursion depths. 
		UINT maxRecursionDepth = 8; // ~ primary rays only. 
		pipelineConfig->Config(maxRecursionDepth);

		TEST(Device::get().get_native_device()->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_dxrStateObject)), L"Couldn't create DirectX Raytracing state object.\n");


		ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
		TEST(m_dxrStateObject.As(&stateObjectProperties));

		//	shader_identifier.assign()
		rayGenShaderIdentifier = identify(stateObjectProperties->GetShaderIdentifier(c_raygenShaderName));
		missShaderIdentifier = identify(stateObjectProperties->GetShaderIdentifier(c_missShaderName));
		missShadowShaderIdentifier = identify(stateObjectProperties->GetShaderIdentifier(L"ShadowMissShader"));

		hitGroupShaderIdentifiers.reserve(materials.size()*2);

	
	

		for (auto& mat : materials)
		{
			{
				closesthit_identifier id;
			id.identifier = identify(stateObjectProperties->GetShaderIdentifier((mat->wshader_name + L"HIT").c_str()));
			id.mat_buffer = mat->get_pixel_buffer()->get_gpu_address();
			hitGroupShaderIdentifiers.push_back(id);
			}
		
			{
				closesthit_identifier id;
				id.identifier = identify(stateObjectProperties->GetShaderIdentifier(L"ShadowClosestHitGroup"));
				id.mat_buffer = 0;
				id.gpu.ptr = 0;
				hitGroupShaderIdentifiers.push_back(id);
			}
		}


	}
	RayTracingShaders()
	{
		signature = RaytracingSignature::create_typed_root();
		signature->set_unfixed(5);
		signature->set_unfixed(6);
	//	signature->set_unfixed(7);

		signature_local = RaytracingLocalSignature::create_typed_root(true);



		blob = *D3D12ShaderCompilerInfo::get().Compile_Shader(FileSystem::get().get_file("raytracing.hlsl")->load_all(), {});
		//	CreateRaytracingPipelineStateObject(blobs);

	}

	void render(MeshRenderContext::ptr context, Render::Texture::ptr& texture, Render::RaytracingAccelerationStructure::ptr scene_as)
	{
		auto timer = context->list->start(L"raytracing");

		auto& _list = context->list;
		// Get shader identifiers.
		UINT  shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;



		auto DispatchRays = [&](auto* commandList, auto* stateObject, auto* dispatchDesc)
		{
			auto m_rayGenShaderTable = _list->place_raw(rayGenShaderIdentifier);
			auto m_hitGroupShaderTable = _list->place_raw(hitGroupShaderIdentifiers);
			auto m_missShaderTable = _list->place_raw(missShaderIdentifier, missShadowShaderIdentifier);

			// Since each shader table has only one shader record, the stride is same as the size.
			dispatchDesc->HitGroupTable.StartAddress = m_hitGroupShaderTable.get_address();
			dispatchDesc->HitGroupTable.SizeInBytes = m_hitGroupShaderTable.size;
			dispatchDesc->HitGroupTable.StrideInBytes = sizeof(closesthit_identifier);

			dispatchDesc->MissShaderTable.StartAddress = m_missShaderTable.get_address();
			dispatchDesc->MissShaderTable.SizeInBytes = m_missShaderTable.size;
			dispatchDesc->MissShaderTable.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

			dispatchDesc->RayGenerationShaderRecord.StartAddress = m_rayGenShaderTable.get_address();
			dispatchDesc->RayGenerationShaderRecord.SizeInBytes = m_rayGenShaderTable.size;
			dispatchDesc->Width = texture->get_size().x;
			dispatchDesc->Height = texture->get_size().y;
			dispatchDesc->Depth = 1;
			commandList->SetPipelineState1(stateObject);
			commandList->DispatchRays(dispatchDesc);
		};



		auto& shader_data = _list->get_compute().set_signature_typed<RaytracingSignature>(signature);

		shader_data.target[0] = texture->texture_2d()->get_uav();
		shader_data.scene = scene_as->get_gpu_address();
		shader_data.camera = context->cam->get_const_buffer();
		shader_data.g_buffer[0] = context->g_buffer->srv_table;

		shader_data.instance_data = instanceData;
		shader_data.vertex_buffers = buffersVertex;
		shader_data.index_buffers = buffersIndex;
	//	shader_data.material_textures = textures;
		//shader_data.mat_const = materials[0]->get_pixel_buffer();
		_list->get_compute().flush_binds(true);



		for (int i = 0; i < materials.size(); i++)
		{
			if (materials[i]->get_texture_handles().empty())
				hitGroupShaderIdentifiers[2 * i].gpu.ptr = 0;
			else
			hitGroupShaderIdentifiers[2 * i].gpu = _list->get_compute().place(materials[i]->get_texture_srvs()).gpu;
		}
		// Bind the heaps, acceleration structure and dispatch rays.
		D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
		{
			//	commandList->SetComputeRootShaderResourceView(GlobalRootSignatureParams::AccelerationStructureSlot, m_topLevelAccelerationStructure->GetGPUVirtualAddress());
			DispatchRays(_list->get_native_list().Get(), m_dxrStateObject.Get(), &dispatchDesc);
		}

	}
};


class triangle_drawer : public GUI::Elements::image
{
	main_renderer::ptr scene_renderer;
	main_renderer::ptr gpu_scene_renderer;

	stencil_renderer::ptr stenciler;

	Render::FrameResourceManager frames;




	GUI::Elements::label::ptr info;

	size_t time = 0;
	ViewPortRenderer last_renderer;
	struct EyeData : public Events::prop_handler
	{

		using ptr = std::shared_ptr<EyeData>;
		G_Buffer g_buffer;
		first_person_camera cam;

		TemporalAA temporal;

		EyeData(Render::RootSignature::ptr sig)
		{
			g_buffer.init(sig);// gpu_meshes_renderer_static->my_signature);

			g_buffer.size.register_change(this, [this](const ivec2& size) {
				temporal.resize(size);

				});
		}
	};

	std::vector<EyeData::ptr> eyes;
	first_person_camera cam;
public:
	using ptr = std::shared_ptr<triangle_drawer>;
	PostProcessGraph::ptr render_graph;


	Variable<bool> enable_gi = Variable<bool>(true, "enable_gi");
	Variable<bool> debug_draw = Variable<bool>(false, "debug_draw");
	//	VoxelGI::ptr voxel_renderer;

	int visible_count;
	debug_drawer drawer;
	mesh_renderer::ptr meshes_renderer;
	gpu_cached_renderer::ptr gpu_meshes_renderer_static;
	gpu_cached_renderer::ptr gpu_meshes_renderer_dynamic;

	Scene::ptr scene;
	Render::QueryHeap::ptr query_heap;
	float draw_time;
	std::shared_ptr<LightingNode> lighting;

	Variable<bool> realtime_debug = Variable<bool>(false, "realtime_debug");
	MeshAssetInstance::ptr instance;


	RayTracingShaders shaders;

	Render::RaytracingAccelerationStructure::ptr scene_as;


	// Build acceleration structures needed for raytracing.
	void BuildAccelerationStructures()
	{

		std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instances;

		scene->iterate([&instances, this](scene_object* object) {
			MeshAssetInstance::ptr mesh = object->get_ptr<MeshAssetInstance>();
			if (mesh)
			{
				auto structures = mesh->create_raytracing_as(gpu_meshes_renderer_static->gpu_nodes.get_gpu()->get_gpu_address());

				for (auto& e : structures)
				{
					int material_id = 0;
					MaterialAsset::ref& material  = mesh->overrided_material[e->material];
					materials::universal_material::ptr universal = material->get_ptr<materials::universal_material>();
					if (universal)
					{

				
						for (material_id=0; material_id < shaders.materials.size(); material_id++)
						{
							if (shaders.materials[material_id] == universal)
							{
								break;
							}
						}

						if (material_id == shaders.materials.size())
						{
							shaders.materials.push_back(universal);
						}
							
							//	shaders.material = universal;

					//		break;
					}

					auto& instance_data = shaders.instanceData[shaders.instanceData.get_free(1)];

					{

						size_t offset = shaders.buffersVertex.get_free(1);

						shaders.buffersVertex[instance_data.vertexBuffer = offset] = e->srvs[0];

					}
					{

						size_t offset = shaders.buffersIndex.get_free(1);

						shaders.buffersIndex[instance_data.indexBuffer = offset] = e->srvs[1];

					}
					D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};

					for (int x = 0; x < 3; x++)
						for (int y = 0; y < 4; y++)
						{
							instanceDesc.Transform[x][y] = mesh->global_transform.rows[y][x];
						}

					
					//instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
					instanceDesc.InstanceMask = 1;
					instanceDesc.AccelerationStructure = e->get_gpu_address();
					instanceDesc.InstanceID = instances.size() + 1;
					instanceDesc.InstanceContributionToHitGroupIndex =  2 * material_id;
					instances.push_back(instanceDesc);
				}

			}

			return true;
			});



		scene_as = std::make_shared<RaytracingAccelerationStructure>(instances);
		shaders.CreateRaytracingPipelineStateObject();
		//	shaders = std::make_shared<RayTracingShaders>(blobs);
	}


	triangle_drawer()
	{

		auto t = CounterManager::get().start_count<triangle_drawer>();
		thinkable = true;
		clickable = true;

		scene.reset(new Scene());
		scene->name = L"Scene";

		query_heap = std::make_shared<  Render::QueryHeap>(2, D3D12_QUERY_HEAP_TYPE::D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS);
		scene_renderer = std::make_shared<main_renderer>();
		scene_renderer->register_renderer(meshes_renderer = std::make_shared<mesh_renderer>(scene));


		gpu_scene_renderer = std::make_shared<main_renderer>();
		gpu_scene_renderer->register_renderer(gpu_meshes_renderer_static = std::make_shared<gpu_cached_renderer>(scene, MESH_TYPE::STATIC));
		gpu_scene_renderer->register_renderer(gpu_meshes_renderer_dynamic = std::make_shared<gpu_cached_renderer>(scene, MESH_TYPE::DYNAMIC));
		cam.position = vec3(0, 5, -30);


		Render::PipelineStateCache::create();
		stenciler.reset(new stencil_renderer());
		stenciler->player_cam = &cam;
		stenciler->scene = scene;
		add_child(stenciler);

		info.reset(new GUI::Elements::label);
		info->docking = GUI::dock::TOP;
		info->x_type = GUI::pos_x_type::LEFT;
		info->magnet_text = FW1_LEFT;
		add_child(info);
		base::ptr props(new base);
		props->docking = GUI::dock::FILL;
		props->x_type = GUI::pos_x_type::LEFT;
		props->y_type = GUI::pos_y_type::TOP;
		props->width_size = GUI::size_type::MATCH_CHILDREN;
		props->height_size = GUI::size_type::MATCH_CHILDREN;
		add_child(props);

		props->add_child(std::make_shared<GUI::Elements::check_box_text>(meshes_renderer->use_parrallel));
		props->add_child(std::make_shared<GUI::Elements::check_box_text>(meshes_renderer->use_cpu_culling));
		props->add_child(std::make_shared<GUI::Elements::check_box_text>(meshes_renderer->use_gpu_culling));
		props->add_child(std::make_shared<GUI::Elements::check_box_text>(meshes_renderer->clear_depth));
		props->add_child(std::make_shared<GUI::Elements::check_box_text>(enable_gi));
		props->add_child(std::make_shared<GUI::Elements::check_box_text>(realtime_debug));


		props->add_child(std::make_shared<GUI::Elements::check_box_text>(debug_draw));

		GUI::Elements::circle_selector::ptr circle(new  GUI::Elements::circle_selector);
		circle->docking = GUI::dock::FILL;
		circle->x_type = GUI::pos_x_type::RIGHT;
		circle->y_type = GUI::pos_y_type::TOP;


		add_child(circle);
		lighting = std::make_shared<LightingNode>();

		circle->on_change.register_handler(this, [this](const float2& value)
			{
				float2 v = value;
				run_on_ui([this, v]() {
					float3 dir = { v.x,sqrt(1.001 - v.length_squared()),-v.y };
					lighting->lighting.pssm.set_position(dir);
					});

			});

		circle->set_value({ 1,0 });



		MeshAsset::ptr asset_ptr = EngineAssets::material_tester.get_asset();

	

		auto make_material = [](float3 color, float roughness, float metallic) {
			MaterialGraph::ptr graph(new MaterialGraph);


			{

				auto value_node = std::make_shared<VectorNode>(vec4(color, 1));
				graph->register_node(value_node);
				value_node->get_output(0)->link(graph->get_base_color());
			}


			{
				auto value_node = std::make_shared<ScalarNode>(roughness);
				graph->register_node(value_node);
				value_node->get_output(0)->link(graph->get_roughness());
			}

			{
				auto value_node = std::make_shared<ScalarNode>(metallic);
				graph->register_node(value_node);
				value_node->get_output(0)->link(graph->get_mettalic());

			}


			materials::universal_material* m = (new materials::universal_material(graph));
			//m->generate_material();
			m->register_new();
			return m->get_ptr<MaterialAsset>();
		};



		auto base_mat = make_material({ 1,1,1 }, 1, 0);

		int count = 1;
		float distance = 5;
		for (int i = 0; i <= count; i++)
			for (int j = 0; j <= count; j++)
			{
				MeshAssetInstance::ptr instance(new MeshAssetInstance(asset_ptr));
				instance->override_material(0, base_mat);
				instance->override_material(1, make_material({ 1,0,0 }, float(i) / count, float(j) / count));

				instance->local_transform[3] = { i * distance,4,j * distance,1 };

				scene->add_child(instance);
			}

		{
			MeshAsset::ptr ruins_ptr = AssetManager::get().find_by_name<MeshAsset>(L"plane.obj");


			if (ruins_ptr)
			{
				MeshAssetInstance::ptr instance(new MeshAssetInstance(ruins_ptr));
				//instance->local_transform.scaling(2);

				//instance->local_transform.a42 = mn.y;
				scene->add_child(instance);
			}

		}


		{
			MeshAsset::ptr ruins_ptr = AssetManager::get().find_by_name<MeshAsset>(L"cubes.obj");


			if (ruins_ptr)
			{
				MeshAssetInstance::ptr instance(new MeshAssetInstance(ruins_ptr));
				instance->local_transform=mat4x4::translation({ 0,3.8,0 });
				scene->add_child(instance);
			}

		}



		render_graph.reset(new PostProcessGraph);
		render_graph->start_child_nodes = true;
		{
			auto adaptation_node = std::make_shared<AdaptationNode>();
			render_graph->register_node(adaptation_node);


			auto sky_node = std::make_shared<SkyNode>();
			render_graph->register_node(sky_node);

			auto sky_render_node = std::make_shared<SkyRenderNode>();
			render_graph->register_node(sky_render_node);


			auto sky_env_node = std::make_shared<SkyEnviromentNode>();
			render_graph->register_node(sky_env_node);


			auto smaa_node = std::make_shared<SMAANode>();
			render_graph->register_node(smaa_node);


			//auto ssgi_node = std::make_shared<SSGINode>();
		//	render_graph->register_node(ssgi_node);


			auto voxel_node = std::make_shared<VoxelNode>();
			render_graph->register_node(voxel_node);


			auto gbuffer_downsampler = std::make_shared<GBufferDownsamplerNode>();
			render_graph->register_node(gbuffer_downsampler);


			render_graph->register_node(lighting);


			sky_node->get_output(0)->link(sky_env_node->get_input(0));


			render_graph->get_input(0)->link(gbuffer_downsampler->get_input(0));

			//	gbuffer_downsampler->get_output(0)->link(sky_render_node->get_input(0));


			gbuffer_downsampler->get_output(0)->link(lighting->get_input(0));
			lighting->get_output(0)->link(voxel_node->get_input(0));
			voxel_node->get_output(0)->link(sky_render_node->get_input(0));


			//	ssgi_node->get_output(0)->link(voxel_node->get_input(0));

			//	ssgi_node->get_output(1)->link(voxel_node->get_input(1));

			sky_env_node->get_output(0)->link(voxel_node->get_input(1));
			sky_node->get_output(0)->link(sky_render_node->get_input(1));

			//voxel_node->get_output(0)->link(sky_render_node->get_input(0));
			//sky_render_node->get_output(0)->link(adaptation_node->get_input(0));
		//	adaptation_node->get_output(0)->link(smaa_node->get_input(0));

			//sky_render_node->get_output(0)->link(smaa_node->get_input(0));
		}






		scene->update_transforms();
		auto list = Device::get().get_upload_list();

		MeshRenderContext::ptr context(new MeshRenderContext());
		context->delta_time = 1;
		context->list = list;

		gpu_meshes_renderer_static->update(context);


		list->end();
		list->execute_and_wait();

		BuildAccelerationStructures();



	}
	float scale_speed = 0;
	vec2 wheel_pos;
	bool on_wheel(mouse_wheel type, float value, vec2 pos)
	{
		scale_speed += value * 0.1f;
		wheel_pos = pos;
		return true;

	}

	void think(float dt)
	{
		//	if (!g_buffer.size.has_value())
			//	return;

		scene->update_transforms();

		auto min = scene->get_min();
		auto max = scene->get_max();


		auto points = cam.get_points(min, max);
		auto bounds = points.get_bounds_in(cam.get_view());

		//cam.set_projection_params(std::max(1.0f, bounds.znear - 1), bounds.zfar + 1);
		cam.set_projection_params(1, 1500);


		cam.frame_move(dt);

	}



	void draw_eye(Render::CommandList::ptr _list, float dt, EyeData& data, Render::Texture::ptr target)
	{


		auto& g_buffer = data.g_buffer;

		auto& cam = data.cam;
		auto& temporal = data.temporal;
		//		auto& last_renderer = data.last_renderer;




		{
			//	auto timer = list->start(L"calculate sizes");

		}

		//	cam.update(temporal.get_current_offset() / *g_buffer.size);






		/*{
		MeshRenderContext::ptr context_gbuffer(new MeshRenderContext(*context));
		context_gbuffer->list = list->get_sub_list();
		thread_pool::get().enqueue([this, context_gbuffer]() {
		auto c = context_gbuffer;
		auto list = c->list;
		list->begin("stenciler");
		stenciler->render(c, scene);
		list->end();
		list->execute();
		});



		}
		*/
		//context->cam = &cam;




	/*	PostProcessContext render_context(scene);
		render_context.g_buffer = &g_buffer;
		render_context.mesh_context = context;

		render_context.realtime_debug = realtime_debug;

		render_context.renderer = gpu_scene_renderer;
		render_graph->owner = render_graph.get();
		render_graph->start(&render_context);
		render_graph->get_input(0)->put(0);
		render_context.wait();


		//		reflect.process(context, g_buffer,nullptr,nullptr);
		{

			list->get_graphics().set_viewport(g_buffer.result_tex.first()->texture_2d()->get_viewport(0));
			list->get_graphics().set_scissor(g_buffer.result_tex.first()->texture_2d()->get_scissor(0));
			list->get_graphics().set_rtv(1, g_buffer.result_tex.first()->texture_2d()->get_rtv(0), Render::Handle());

			//	if (debug_draw)  drawer.draw(*list, &cam);
			//	stenciler->draw_after(context, g_buffer);
		}

		{
			temporal.make_current(g_buffer.result_tex.first());
			//dont insert here anything
			last_renderer.process(context, temporal, target);
		//	last_renderer.process(context, temporal, vr.eyes[1].color_buffer);
		}
		list->transition(g_buffer.result_tex.first(), Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);

		visible_count = context->draw_count;*/

	}

	void update_texture(Render::CommandList::ptr _list, float dt, const std::shared_ptr<OVRContext>& vr)
	{


		auto& list = frames.set_frame(_list, "game window");
		MeshRenderContext::ptr context(new MeshRenderContext());
		context->delta_time = dt;
		//	context->g_buffer = &g_buffer;
		time += static_cast<size_t>(dt * 1000.0f);
		context->current_time = time;
		context->sky_dir = lighting->lighting.pssm.get_position();
		context->priority = TaskPriority::HIGH;
		context->list = list;

		context->eye_context = vr;

		{

			MeshRenderContext::ptr context_gbuffer(new MeshRenderContext(*context));
			context_gbuffer->list = list->get_sub_list();
			//	thread_pool::get().enqueue([this, &g_buffer, context_gbuffer]() {
			auto c = context_gbuffer;
			auto list = c->list;
			list->begin("update");

			gpu_meshes_renderer_static->update(context_gbuffer);
			gpu_meshes_renderer_dynamic->update(context_gbuffer);

			list->end();
			list->execute();
			//	});




			//	if (debug_draw) scene->debug_draw(drawer);

		}




		while (eyes.size() < vr->eyes.size())
		{
			eyes.emplace_back(new EyeData(gpu_meshes_renderer_static->my_signature));
			eyes.back()->g_buffer.size = get_render_bounds().size;
		}
		//	eyes.resize(vr.eyes.size());



		for (int i = 0; i < eyes.size(); i++)
		{
			eyes[i]->cam = cam;

			eyes[i]->cam.eye_rot = vr->eyes[i].dir;
			eyes[i]->cam.offset = vr->eyes[i].offset;
			context->eye_context->eyes[i].g_buffer = &eyes[i]->g_buffer;
			context->eye_context->eyes[i].cam = &eyes[i]->cam;

			//	eyes[i]->g_buffer=



			//TODO: VR SWITHC HEREEEEEEEEEEEEEEE
		//	eyes[i]->cam.set_projection_params(eyeRenderDesc[i].Fov, 0.2f, 1000.0f);
			eyes[i]->cam.update(eyes[i]->temporal.get_current_offset() / *eyes[i]->g_buffer.size);




			//	draw_eye(_list,dt, *eyes[i], vr.eyes[i].color_buffer);

			auto& g_buffer = eyes[i]->g_buffer;
			auto& temporal = eyes[i]->temporal;
			//	auto& last_renderer = eyes[i]->last_renderer;
			auto& target = vr->eyes[i].color_buffer;


			g_buffer.reset(context);
			context->g_buffer = &g_buffer;
			context->cam = &eyes[i]->cam;

			{
				context->cam = &eyes[i]->cam;

				MeshRenderContext::ptr context_gbuffer(new MeshRenderContext(*context));
				context_gbuffer->list = list->get_sub_list();
				thread_pool::get().enqueue([this, &g_buffer, context_gbuffer]() {
					auto c = context_gbuffer;
					auto list = c->list;
					list->begin("gbuffer creating");

					g_buffer.set(c, true);
					gpu_scene_renderer->render(c, scene);
					g_buffer.end(c);
					list->end();
					list->execute();
					});




				//	if (debug_draw) scene->debug_draw(drawer);

			}


		}

		{
			stenciler->player_cam = &eyes[0]->cam;

			MeshRenderContext::ptr context_gbuffer(new MeshRenderContext(*context));
			context_gbuffer->list = list->get_sub_list();
			thread_pool::get().enqueue([this, context_gbuffer]() {
				auto c = context_gbuffer;
				auto list = c->list;
				list->begin("stenciler");
				stenciler->render(c, scene);
				list->end();
				list->execute();
				});
		}


		PostProcessContext render_context(scene);
		render_context.g_buffer = &eyes[0]->g_buffer;
		render_context.mesh_context = context;

		render_context.realtime_debug = realtime_debug;

		render_context.renderer = gpu_scene_renderer;
		render_graph->owner = render_graph.get();
		render_graph->start(&render_context);
		render_graph->get_input(0)->put(0);
		render_context.wait();

		stenciler->draw_after(context, eyes[0]->g_buffer);

		for (auto& eye : eyes)
			eye->g_buffer.result_tex.swap(list);


		/// HERE NEED TO SWAP OCULUS TO 2D
		//if (!context->eye_context->eyes[0].color_buffer)
		{



			context->eye_context->eyes[0].color_buffer = texture.texture;// eyes[0]->g_buffer.result_tex.first();
		}



		{

			//temporal.make_current(g_buffer.result_tex.first());
			//dont insert here anything
			last_renderer.process(context);
			//last_renderer.process(context, temporal);


			//	last_renderer.process(context, temporal, vr.eyes[1].color_buffer);
		}

		list->transition(texture.texture, Render::ResourceState::UNORDERED_ACCESS);
		list->flush_transitions();

		shaders.render(context, texture.texture, scene_as);

	}

	virtual void draw(Render::context& t) override
	{

		//	auto r = get_render_bounds();
		//	cam.set_projection_params(pi / 4, float(r.w) / r.h, 1, 1500);
		//	for (auto& e : eyes)
		//		e->g_buffer.size = { r.w,r.h };

	//	renderer->flush(t);
		t.command_list->transition(texture.texture, Render::ResourceState::PIXEL_SHADER_RESOURCE);

		image::draw(t);
		//	renderer->flush(t);

		//	texture.srv[0] = t.command_list->get_graphics().get_desc_manager().get(2, 0);
		//	if(eyes.size())
		//		texture = eyes[0]->g_buffer.result_tex.first();

		//	Handle h = t.command_list->get_graphics().get_desc_manager().get(7, 0);
		float dt = t.delta_time;
		auto list = t.command_list->get_sub_list();

		auto v = t.ovr_context;
		thread_pool::get().enqueue([this, dt, list, v]() {
			//	

			list->begin("__draw");
			list->transition(texture.texture, Render::ResourceState::RENDER_TARGET);
			list->clear_rtv(texture.texture->texture_2d()->get_rtv());

			update_texture(list, dt, v);
			//	texture = eyes[0]->g_buffer.result_tex.first();

		//	h.place(eyes[0]->g_buffer.result_tex.first()->texture_2d()->get_static_srv());  //;'///////////////////////////////////////////////////////////

			list->end();
			list->execute();
			//		h.place(g_buffer.speed_tex->texture_2d()->get_static_srv());
				//	h.place(g_buffer.depth_tex_downscaled->texture_2d()->get_static_srv());
			//h.place(lighting.pssm.screen_light_mask->texture_2d()->get_static_srv());
	//		h.place(voxel_renderer->downsampled_light->texture_2d()->get_static_srv());
		//	h.place(hdr.tex_luma->texture_2d()->get_static_srv());
			});

		//	texture = eyes[0]->g_buffer.result_tex.first();

	}





	virtual void on_bounds_changed(const rect& r) override
	{
		base::on_bounds_changed(r);
		//	std::this_thread::sleep_for(1s);
		if (r.w <= 64 || r.h <= 64) return;

		ivec2 size = r.size;
		texture.texture.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, size.x, size.y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), Render::ResourceState::PIXEL_SHADER_RESOURCE));


		//	g_buffer.size = { r.w, r.h };
		cam.set_projection_params(pi / 4, float(r.w) / r.h, 1, 1500);
		for (auto& e : eyes)
			e->g_buffer.size = { r.w,r.h };
	}



};

class tick_timer
{
	std::chrono::time_point<std::chrono::system_clock> last_tick;

public:
	tick_timer()
	{
		last_tick = std::chrono::system_clock::now();
	}

	double tick()
	{
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = now - last_tick;
		last_tick = now;
		return elapsed_seconds.count();
	}
};

class count_meter
{
	double time = 0;
	tick_timer t;
	unsigned int ticks = 0;

	double average = 0;
public:

	bool tick()
	{
		time += t.tick();
		ticks++;

		if (time > 1)
		{
			average = ticks / time;
			ticks = 0;
			time = 0;
			return true;
		}

		return false;
	}

	float get()
	{
		return (float)average;
	}
};




class UIWindow : public Window, public GUI::user_interface
{
	Render::SwapChain::ptr swap_chain;

	tick_timer main_timer;
	ivec2 new_size;

	std::shared_ptr<OVRContext> vr_context = std::make_shared<OVRContext>();
	std::future<void> task_future;
protected:
	virtual	void render()
	{

		{
			auto& timer = Profiler::get().start(L"start new frame");
			swap_chain->start_next();

		}


		GUI::user_interface::size = new_size;
		auto& timer = Profiler::get().start(L"draw");

		auto command_list = Render::Device::get().get_queue(Render::CommandListType::DIRECT)->get_free_list();

		{
			command_list->begin("main window", &timer);

			command_list->transition(swap_chain->get_current_frame(), Render::ResourceState::RENDER_TARGET);
			command_list->get_graphics().set_rtv(1, swap_chain->get_current_frame()->texture_2d()->get_rtv(), Render::Handle());
			command_list->get_graphics().set_viewports({ swap_chain->get_current_frame()->texture_2d()->get_viewport() });

			std::shared_ptr<Render::CommandList> label_list;
			Render::context render_context(command_list, label_list, vr_context);
			render_context.delta_time = static_cast<float>(main_timer.tick());

			render_context.ovr_context->eyes.resize(1);
			render_context.ovr_context->eyes[0].dir = quat();

			render_context.ovr_context->eyes[0].offset = vec3(0, 0, 0);
			//		render_context.ovr_context.eyes[0].color_buffer = swap_chain->get_current_frame();


			{
				auto timer = command_list->start(L"draw ui");
				draw_ui(render_context);
			}
			command_list->transition(swap_chain->get_current_frame(), Render::ResourceState::PRESENT);
			command_list->end();
		}

		{
			auto& timer = Profiler::get().start(L"execute");


			command_list->when_send([this](UINT64 res) {
				auto& timer = Profiler::get().start(L"present");

				swap_chain->present(res);
				if (Application::get().is_alive())
				{
					auto ptr = get_ptr();
					task_future = scheduler::get().enqueue([ptr, this]() {
						render();
						}, std::chrono::steady_clock::now());
				}

				});
			command_list->execute();
		}
	}

	UIWindow()
	{
		Window::input_handler = this;
		DX11::swap_chain_desc desc;
		desc.format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.fullscreen = false;
		desc.stereo = false;
		desc.window = this;
		swap_chain = Render::Device::get().create_swap_chain(desc);



		set_capture = [this](bool v)
		{
			if (v)
				SetCapture(get_hwnd());
			else
				ReleaseCapture();
		};

		//	GUI::user_interface::size.register_handler(this, [this](ivec2 size) {	if (swap_chain)	swap_chain->resize(size); });
	}

	virtual ~UIWindow()
	{

		if (task_future.valid())
			task_future.wait();
	}
	void on_resize(vec2 size) override
	{
		new_size = size;
	}


	virtual void on_size_changed(const vec2& r) override
	{
		user_interface::on_size_changed(r);
		if (swap_chain)	swap_chain->resize(r);
	}
};
#ifdef OCULUS_SUPPORT

//------------------------------------------------------------
// ovrSwapTextureSet wrapper class that also maintains the render target views
// needed for D3D12 rendering.
struct OculusEyeTexture
{
	ovrSession               Session;
	ovrTextureSwapChain      TextureChain;
	ovrTextureSwapChain      DepthTextureChain;

	//	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> TexRtv;
	std::vector<Render::Texture::ptr> TexResource;

	//std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> DepthTexDsv;
	//std::vector<ID3D12Resource*>             DepthTex;

	OculusEyeTexture() :
		Session(nullptr),
		TextureChain(nullptr),
		DepthTextureChain(nullptr)
	{
	}

	bool Init(ovrSession session, int sizeW, int sizeH)
	{
		Session = session;

		ovrTextureSwapChainDesc desc = {};
		desc.Type = ovrTexture_2D;
		desc.ArraySize = 1;
		desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.Width = sizeW;
		desc.Height = sizeH;
		desc.MipLevels = 1;
		desc.SampleCount = 1;
		desc.MiscFlags = ovrTextureMisc_DX_Typeless | ovrTextureMisc_AutoGenerateMips;
		desc.StaticImage = ovrFalse;
		desc.BindFlags = ovrTextureBind_DX_RenderTarget;

		ovrResult result = ovr_CreateTextureSwapChainDX(session, Render::Device::get().get_queue(Render::CommandListType::DIRECT)->get_native().Get(), &desc, &TextureChain);
		if (!OVR_SUCCESS(result))
			return false;

		int textureCount = 0;
		ovr_GetTextureSwapChainLength(Session, TextureChain, &textureCount);
		//TexRtv.resize(textureCount);
		TexResource.resize(textureCount);
		for (int i = 0; i < textureCount; ++i)
		{

			ComPtr<ID3D12Resource> res;
			result = ovr_GetTextureSwapChainBufferDX(Session, TextureChain, i, IID_PPV_ARGS(&res));
			if (!OVR_SUCCESS(result))
				return false;
			TexResource[i] = std::make_shared<Texture>(res);

			//	TexResource[i]->SetName(L"EyeColorRes");






			D3D12_RENDER_TARGET_VIEW_DESC rtvd = {};
			rtvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			rtvd.ViewDimension = (1 > 1) ? D3D12_RTV_DIMENSION_TEXTURE2DMS
				: D3D12_RTV_DIMENSION_TEXTURE2D;
			//	TexRtv[i] = DIRECTX.RtvHandleProvider.AllocCpuHandle();
			//	DIRECTX.Device->CreateRenderTargetView(TexResource[i], &rtvd, TexRtv[i]);
		}

		return true;
	}
	Render::Texture::ptr GetD3DColorResource()
	{
		int index = 0;
		ovr_GetTextureSwapChainCurrentIndex(Session, TextureChain, &index);
		return TexResource[index];
	}
	~OculusEyeTexture()
	{
		if (TextureChain)
		{

			ovr_DestroyTextureSwapChain(Session, TextureChain);
		}


	}



	// Commit changes
	void Commit()
	{
		ovr_CommitTextureSwapChain(Session, TextureChain);


	}
};
#endif
UINT64 frame_counter = 0;
#ifdef OCULUS_SUPPORT
class OVRRender
{
public:

	tick_timer main_timer;

	using ptr = std::shared_ptr<OVRRender>;
	triangle_drawer::ptr drawer;

	long long frameIndex = 0;



	Render::SwapChain::ptr swap_chain;

	ovrSession session;
	OculusEyeTexture* pEyeRenderTexture[2] = { nullptr, nullptr };
	// Setup VR components, filling out description
	ovrRecti eyeRenderViewport[2];

	ovrHmdDesc hmdDesc;

	std::shared_ptr<OVRContext> vr_context = std::make_shared<OVRContext>();
	virtual	void render()
	{


		ovrSessionStatus sessionStatus;
		ovr_GetSessionStatus(session, &sessionStatus);
		if (sessionStatus.ShouldQuit)
		{
			// Because the application is requested to quit, should not request retry
	//		retryCreate = false;
			return;
		}
		if (sessionStatus.ShouldRecenter)
			ovr_RecenterTrackingOrigin(session);

		if (sessionStatus.IsVisible)
		{

			// Get both eye poses simultaneously, with IPD offset already included. 
			ovrPosef EyeRenderPose[2];
			ovrPosef HmdToEyePose[2] = { eyeRenderDesc[0].HmdToEyePose, eyeRenderDesc[1].HmdToEyePose };

			double sensorSampleTime;    // sensorSampleTime is fed into the layer later;lkjiwwwwjlwjdjjjjjjjj
			ovr_GetEyePoses(session, frameIndex, ovrTrue, HmdToEyePose, EyeRenderPose, &sensorSampleTime);
			auto command_list = Render::Device::get().get_queue(Render::CommandListType::DIRECT)->get_free_list();


			command_list->begin("main window");
			std::shared_ptr<Render::CommandList> label_list;

			Render::context render_context(command_list, label_list, vr_context);

			// Render Scene to Eye Buffers
			for (int eye = 0; eye < 2; ++eye)
			{

				auto tex = pEyeRenderTexture[eye]->GetD3DColorResource();

				command_list->transition(tex, Render::ResourceState::RENDER_TARGET);
				//command_list->get_graphics().set_rtv(1, tex->texture_2d()->get_rtv(), Render::Handle());
				//command_list->get_graphics().set_viewports({ tex->texture_2d()->get_viewport() });
				command_list->clear_rtv(tex->texture_2d()->get_rtv(), { eye,0,0,1 });




			}
			vr_context->eyes.resize(2);

			vr_context->eyes[0].dir = { EyeRenderPose[0].Orientation.x, EyeRenderPose[0].Orientation.y,
				-EyeRenderPose[0].Orientation.z, EyeRenderPose[0].Orientation.w };
			vr_context->eyes[0].offset = { EyeRenderPose[0].Position.x, EyeRenderPose[0].Position.y,
				-EyeRenderPose[0].Position.z };


			vr_context->eyes[1].dir = { EyeRenderPose[1].Orientation.x, EyeRenderPose[1].Orientation.y,
				-EyeRenderPose[1].Orientation.z, EyeRenderPose[1].Orientation.w };

			vr_context->eyes[1].offset = { EyeRenderPose[1].Position.x, EyeRenderPose[1].Position.y,
				-EyeRenderPose[1].Position.z };

			//	render_context.ovr_context.eyes[0].fov = eyeRenderDesc[0].Fov.;
			//render_context.ovr_context.eyes[1].fov = eyeRenderDesc[1].Fov;




			vr_context->eyes[0].color_buffer = pEyeRenderTexture[0]->GetD3DColorResource();
			vr_context->eyes[1].color_buffer = pEyeRenderTexture[1]->GetD3DColorResource();

			render_context.delta_time = static_cast<float>(main_timer.tick());


			{
				//	auto timer = command_list->start(L"draw ui");
			//		drawer->draw(render_context);
				drawer->think(render_context.delta_time);
				drawer->update_texture(command_list, render_context.delta_time, vr_context);
			}


			for (int eye = 0; eye < 2; ++eye)
			{

				auto tex = pEyeRenderTexture[eye]->GetD3DColorResource();

				command_list->transition(tex, Render::ResourceState::PRESENT);


			}
			//		command_list->execute_and_wait();



			command_list->end();

			// Initialize our single full screen Fov layer.
			ovrLayerEyeFov ld = {};
			ld.Header.Type = ovrLayerType_EyeFov;
			ld.Header.Flags = 0;

			for (int eye = 0; eye < 2; ++eye)
			{
				ld.ColorTexture[eye] = pEyeRenderTexture[eye]->TextureChain;
				ld.Viewport[eye] = eyeRenderViewport[eye];
				ld.Fov[eye] = hmdDesc.DefaultEyeFov[eye];
				ld.RenderPose[eye] = EyeRenderPose[eye];
				ld.SensorSampleTime = sensorSampleTime;
			}

			command_list->when_send([this, ld](UINT64 res) {

				pEyeRenderTexture[0]->Commit();
				pEyeRenderTexture[1]->Commit();



				const	ovrLayerHeader* layers = &ld.Header;

				ovr_SubmitFrame(session, frameIndex, nullptr, &layers, 1);



				frameIndex++;

				scheduler::get().enqueue(std::bind(&OVRRender::render, this), std::chrono::steady_clock::now());
				});
			command_list->execute();

		}
		else
			scheduler::get().enqueue(std::bind(&OVRRender::render, this), std::chrono::steady_clock::now());


		Profiler::get().on_frame(frame_counter++);

		if (GetAsyncKeyState('R'))
		{
			//   AssetManager::get().reload_resources();
			Render::pixel_shader::reload_all();
			Render::vertex_shader::reload_all();
			Render::geometry_shader::reload_all();
			Render::hull_shader::reload_all();
			Render::domain_shader::reload_all();
			Render::compute_shader::reload_all();
			Render::Texture::reload_all();
		}
		{
			auto& timer = Profiler::get().start(L"AssetManager");
			AssetManager::get().tact();
		}
		//		DIRECTX.Run(MainLoop);

	}



	OVRRender()
	{


		drawer = std::make_shared<triangle_drawer>();

		GUI::Elements::image::ptr back(new GUI::Elements::image);
		back->texture = Render::Texture::get_resource(Render::texure_header("textures/gui/background2.jpg", false, false));
		back->texture.tiled = true;
		back->width_size = GUI::size_type::MATCH_PARENT;
		back->height_size = GUI::size_type::MATCH_PARENT;
		//	iface->add_child(back);
		ovrMirrorTextureDesc        mirrorDesc = {};

		// Initializes LibOVR, and the Rift
		ovrInitParams initParams = { ovrInit_RequestVersion | ovrInit_FocusAware, OVR_MINOR_VERSION, NULL, 0, 0 };
		ovrResult result = ovr_Initialize(&initParams);
		if (!OVR_SUCCESS(result))
			return;

		ovrGraphicsLuid luid;
		result = ovr_Create(&session, &luid);
		if (!OVR_SUCCESS(result))
			return;

		hmdDesc = ovr_GetHmdDesc(session);







		for (int eye = 0; eye < 2; ++eye)
		{
			ovrSizei idealSize = ovr_GetFovTextureSize(session, (ovrEyeType)eye, hmdDesc.DefaultEyeFov[eye], 1.0f);
			pEyeRenderTexture[eye] = new OculusEyeTexture();
			if (!pEyeRenderTexture[eye]->Init(session, idealSize.w, idealSize.h))
			{
				//	if (retryCreate) goto Done;
				//	FATALERROR("Failed to create eye texture.");
			}

			eyeRenderViewport[eye].Pos.x = 0;
			eyeRenderViewport[eye].Pos.y = 0;
			eyeRenderViewport[eye].Size = idealSize;
			if (!pEyeRenderTexture[eye]->TextureChain)
			{
				//	if (retryCreate) goto Done;
				//	FATALERROR("Failed to create texture.");
			}

			drawer->size = { idealSize.w, idealSize.h };

		}

		drawer->update_layout({ 0,0,drawer->size.get() }, 1);

		eyeRenderDesc[0] = ovr_GetRenderDesc(session, ovrEye_Left, hmdDesc.DefaultEyeFov[0]);
		eyeRenderDesc[1] = ovr_GetRenderDesc(session, ovrEye_Right, hmdDesc.DefaultEyeFov[1]);
		/*	// Create a mirror to see on the monitor.
			mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
			mirrorDesc.Width = 256;
			mirrorDesc.Height = 256;
			mirrorDesc.MiscFlags = ovrTextureMisc_None;
			result = ovr_CreateMirrorTextureDX(session, DIRECTX.CommandQueue, &mirrorDesc, &mirrorTexture);
			if (!OVR_SUCCESS(result))
			{
				if (retryCreate) goto Done;
				FATALERROR("Failed to create mirror texture.");
			}

			*/





	}
	~OVRRender()
	{

		ovr_Shutdown();
	}
};
#endif
class WindowRender : public UIWindow
{
	count_meter fps;

	GUI::Elements::label::ptr label_fps;
	GUI::Elements::label::ptr label_tiles;
	GUI::Elements::label::ptr instance_info;

	std::shared_ptr<triangle_drawer> drawer;

	GUI::base::ptr area;
	mouse::ptr pMouse;

public:




	WindowRender()
	{
		Profiler::create();
		EVENT("Start WindowRender");

		EngineAssets::brdf.get_asset();

		GUI::Elements::image::ptr back(new GUI::Elements::image);
		back->texture = Render::Texture::get_resource(Render::texure_header("textures/gui/back_fill.png", false, false));
		back->texture.tiled = true;
		back->width_size = GUI::size_type::MATCH_PARENT;
		back->height_size = GUI::size_type::MATCH_PARENT;
		add_child(back);
		//  GUI::Elements::check_box::ptr butt(new GUI::Elements::check_box());
		//   butt->docking = GUI::dock::FILL;
		//   ui->add_child(butt);

		on_resize(get_size());
		EVENT("Start UI");
		{
			area.reset(new GUI::base());
			area->docking = GUI::dock::FILL;
			add_child(area);
			GUI::Elements::dock_base::ptr  d = std::make_shared<GUI::Elements::dock_base>();
			d->docking = GUI::dock::FILL;
			{
				EVENT("Start Drawer");
				drawer.reset(new triangle_drawer());
				drawer->docking = GUI::dock::FILL;
				//	area->add_child(drawer);
				d->get_tabs()->add_page("Game", drawer);
				EVENT("End Drawer");
			}
			{
				GUI::Elements::list_box::ptr l(new GUI::Elements::list_box());
				auto& dock = d->get_dock(GUI::dock::BOTTOM);
				dock->size = { 100, 100 };
				area->add_child(d);
				dock->get_tabs()->add_page("TaskViewer", std::make_shared<GUI::Elements::Debug::TaskViewer>());
				//			dock->get_tabs()->add_page("output", std::make_shared<GUI::Elements::Debug::OutputWindow>());
							//       GUI::Elements::tree::ptr t(new GUI::Elements::tree());
			//                    t->init(drawer->scene.get());
			//                    dock->get_tabs()->add_page("Scene", t);
				{
					//	GUI::Elements::Debug::TimerWatcher::ptr t(new GUI::Elements::Debug::TimerWatcher());
					//	t->init(&Profiler::get());
					//	dock->get_tabs()->add_page("Profiler", t);

					GUI::Elements::Debug::TimeGraph::ptr t2(new GUI::Elements::Debug::TimeGraph());

					dock->get_tabs()->add_page("Graph", t2);


				}
			}
			{
				{
					GUI::Elements::menu_strip::ptr menu(new GUI::Elements::menu_strip());
					auto file = menu->add_item("File")->get_menu();
					auto edit = menu->add_item("Edit")->get_menu();
					auto help = menu->add_item("Help");// ->get_menu();
					file->add_item("New");
					file->add_item("Open")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
					{
						add_task([this]()
							{
								try
								{
									auto f = FileSystem::get().get_file("scene.dat")->load_all();

									Scene::ptr scene(new Scene());

									Serializer::deserialize(f, *scene);
									//	drawer->scene = scene;
								}
								catch (std::exception e)
								{
									message_box("error", "cant open", [](bool) {});
								}


							});
					};
					file->add_item("Save")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
					{
						//	auto data = Serializer::serialize(*drawer->scene);
						//	FileSystem::get().save_data(L"scene.dat", data);
					};
					file->add_item("Quit")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
					{
						on_destroy();
					};
					auto add = edit->add_item("Add smth")->get_menu();
					add->add_item("Mesh");
					add->add_item("Material");
					add->add_item("Sound");
					//->get_menu()->add_item("12454");
					add_child(menu);
				}
				{
					GUI::Elements::status_bar::ptr bar(new GUI::Elements::status_bar());
					label_fps = GUI::Elements::label::ptr(new GUI::Elements::label());
					instance_info = GUI::Elements::label::ptr(new GUI::Elements::label());
					label_tiles = GUI::Elements::label::ptr(new GUI::Elements::label());

					bar->add_child(label_fps);
					bar->add_child(label_tiles);

					bar->add_child(instance_info);
					instance_info->docking = GUI::dock::RIGHT;
					label_tiles->margin = { 20,0,0,0 };
					add_child(bar);
					/*

					if (false&&drawer)
					{
						drawer->gpu_meshes_renderer->statistic.enabled = true;
						{
							auto param = std::make_shared<GUI::Elements::label>();
							param->docking = GUI::dock::RIGHT;
							param->margin = { 20,0,0,0 };

						bar->add_child(param);

						auto f = param.get();
						drawer->gpu_meshes_renderer->statistic.frustum_visible_commands.register_handler(f, [f](const int &v) {
							f->text = std::to_string(v);
						});

						}


						{
							auto param = std::make_shared<GUI::Elements::label>();
							param->docking = GUI::dock::RIGHT;
							param->margin = { 20,0,0,0 };

							bar->add_child(param);

							auto f = param.get();
							drawer->gpu_meshes_renderer->statistic.second_stage_test.register_handler(f, [f](const int &v) {
								f->text = std::to_string(v);
							});

						}


						{
							auto param = std::make_shared<GUI::Elements::label>();
							param->docking = GUI::dock::LEFT;
							param->margin = { 20,0,0,0 };

							bar->add_child(param);

							auto f = param.get();
							drawer->gpu_meshes_renderer->statistic.draw_infos.register_handler(f, [f](const gpu_cached_renderer::DrawInfo &v) {
								std::string result;
								for (auto s : v)
								{
									result += std::to_string(s.first)+" ";
									for (auto di :s.second)
									{
										result += std::to_string(di.first) + ":"+ std::to_string(di.second)+"| " ;

									}

									result +=  "|| ";
								}
								f->text = result;
							});

						}



					}*/
				}
				//Log::get()<<Log::LEVEL_ERROR << "Не удается найти указанный файл" << Log::endl;
				auto dock = d->get_dock(GUI::dock::LEFT);
				//            GUI::Elements::FlowGraph::manager::ptr m(new GUI::Elements::FlowGraph::manager());
				auto m = GUI::Elements::FlowGraph::manager::get().get_ptr<GUI::Elements::FlowGraph::manager>();
				//    dock->get_tabs()->add_button(m->add_graph(g));
				dock->get_tabs()->add_button(m->create_parameter_view());
				add_child(m);
				dock->size = { 400, 400 };
				{
					EVENT("Start Asset Explorer");
					auto dock = d->get_dock(GUI::dock::RIGHT);
					GUI::Elements::asset_explorer::ptr cont(new GUI::Elements::asset_explorer());
					dock->get_tabs()->add_page("Asset Explorer", cont);
					dock->size = { 400, 400 };
					EVENT("End Asset Explorer");
				}


				{
					GUI::Elements::window::ptr wnd(new GUI::Elements::window);
					add_child(wnd);
					GUI::Elements::dock_base::ptr dock(new GUI::Elements::dock_base);
					wnd->add_child(dock);
					dock->get_tabs()->add_button(GUI::Elements::FlowGraph::manager::get().add_graph(drawer->render_graph));
					wnd->pos = { 200, 200 };
					wnd->size = { 300, 300 };
				}

			}
		}
		/*        properties = PropertyWindow::ptr(new PropertyWindow());
				properties->pos = { 70, 70 };
				properties->size = { 170, 170 };
				properties->docking = GUI::dock::NONE;
				area->add_child(GUI::Elements::window::ptr(properties));*/
	}

	UINT64 frame_counter = 0;

	virtual void draw_ui(Render::context& c) override
	{
		{
			auto timer = c.command_list->start(L"read timings");
			auto ptr = get_ptr();
			Render::GPUTimeManager::get().read_buffer(c.command_list, [ptr, this]() {
				run_on_ui([]() {	Profiler::get().update(); });

				});
		}

		user_interface::draw_ui(c);

	}
	virtual void render() override
	{
		Profiler::get().on_frame(frame_counter++);

		if (GetAsyncKeyState('R'))
		{
			//   AssetManager::get().reload_resources();
			Render::pixel_shader::reload_all();
			Render::vertex_shader::reload_all();
			Render::geometry_shader::reload_all();
			Render::hull_shader::reload_all();
			Render::domain_shader::reload_all();
			Render::compute_shader::reload_all();
			Render::Texture::reload_all();
		}
		{
			auto& timer = Profiler::get().start(L"AssetManager");
			AssetManager::get().tact();
		}


		if (fps.tick())
		{
			label_fps->text = std::to_string(fps.get());

		}

		UIWindow::render();

	}
	void on_destroy() override
	{
		Application::get().shutdown();
	}

};


class RenderApplication : public Application
{

	friend class Singleton<Application>;

	std::shared_ptr<WindowRender> main_window;
#ifdef OCULUS_SUPPORT
	std::shared_ptr<OVRRender> ovr;
#endif
	// TODO:: shared_objects
	std::atomic<bool> need_reload_resource;
	std::atomic<bool> need_print_screen;

protected:
	RenderApplication()
	{
		EVENT("Device");
		Render::Device::create();
		EVENT("input_system");
		input_system::create();
		FileSystem::get().register_provider(std::make_shared<native_file_provider>());
		EVENT("AssetManager");
		AssetManager::create();
		EVENT("WindowRender");
#ifdef OCULUS_SUPPORT
		//ovr = std::make_shared<OVRRender>();
#endif

		main_window = std::make_shared<WindowRender>();


		create_task([this]() {

			if (main_window)main_window->render();

#ifdef OCULUS_SUPPORT
			if (ovr)ovr->render();
#endif

			});
		EVENT("good");
	}

	virtual ~RenderApplication()
	{
		shutdown();// really need this?

		main_window = nullptr;

		std::this_thread::sleep_for(100ms);
		scheduler::reset();
		//


	//	
		//   Render::Device::get().get_queue(Render::CommandListType::DIRECT)->stop_all();
		Render::Device::get().stop_all();
		Skin::reset();
		Render::Texture::reset_manager();
		Render::pixel_shader::reset_manager();
		Render::vertex_shader::reset_manager();
		Render::domain_shader::reset_manager();
		Render::hull_shader::reset_manager();
		Render::geometry_shader::reset_manager();
		Render::compute_shader::reset_manager();
		GUI::Elements::FlowGraph::manager::reset();
		Profiler::reset();
		Render::GPUTimeManager::reset();
		///    main_window2 = nullptr;
		Fonts::FontSystem::reset();
		AssetRenderer::reset();
		Render::BufferCache::reset();
		TextureAssetRenderer::reset();
		AssetManager::reset();
		Render::Device::reset();
		//   Render::Device::reset();
	}


	virtual void on_tick() override
	{
		if (GetAsyncKeyState(VK_ESCAPE))
			shutdown();

		Window::process_messages();
	}

};

void SetupDebug()
{
	Log::create<WinErrorLogger>();
	FileTXTLogger::create();
	VSOutputLogger::create();
	//  ConsoleLogger::create();
	Log::get().set_logging_level(Log::LEVEL_ALL);
	// Here we can disable some of notification types
	ClassLogger<resource_system>::get().set_logging_level(Log::LEVEL_ALL);
	ClassLogger<DX11::dxgi_system>::get().set_logging_level(Log::LEVEL_ALL);
	ClassLogger<Resource>::get().set_logging_level(Log::LEVEL_ALL);
	ClassLogger<singleton_system>::get().set_logging_level(Log::LEVEL_ALL);
	ClassLogger<Render::Resource>::get().set_logging_level(Log::LEVEL_ALL);
	Log::get() << Log::LEVEL_INFO << "info text" << Log::endl;
	Log::get() << Log::LEVEL_WARNING << "warning text" << Log::endl;
	Log::get() << Log::LEVEL_DEBUG << "debug text" << Log::endl;
	Log::get() << Log::LEVEL_ERROR << "error text" << Log::endl;
}


#include <DbgHelp.h>
int APIENTRY WinMain(_In_ HINSTANCE hinst,
	_In_opt_ HINSTANCE,
	_In_ LPTSTR,
	_In_ int)
{
	setlocale(LC_ALL, "");
	CoInitialize(NULL);
	static	HANDLE process = GetCurrentProcess();
	SymInitialize(process, NULL, TRUE);



	//entry->SetCompressionStream();
	//	archive->WriteToStream();
//    FlowGraph::FlowSystem::get().register_node<FlowGraph::graph>("empty graph");
//   FlowGraph::FlowSystem::get().register_node<test_node>("test_node");
	// FlowGraph::FlowSystem::get().register_node<start_test_node>("start_test_node");
	// FlowGraph::FlowSystem::get().register_node<test_graph>("ololol");
//   FlowGraph::FlowSystem::get().register_node<TextureNode>("TextureNode");
	FlowGraph::FlowSystem::get().register_node<PowerNode>("PowerNode");
	FlowGraph::FlowSystem::get().register_node<SumNode>("SumNode");
	FlowGraph::FlowSystem::get().register_node<MulNode>("MulNode");
	FlowGraph::FlowSystem::get().register_node<SpecToMetNode>("SpecToMetNode");


	FlowGraph::FlowSystem::get().register_node("Scalar", []()->ScalarNode::ptr
		{
			auto res = std::make_shared<ScalarNode>(1.0f);
			res->name = "Scalar";
			return res;
		});



	FlowGraph::FlowSystem::get().register_node("ZeroColor", []()->VectorNode::ptr
		{
			auto res = std::make_shared<VectorNode>(float4{ 0,0,0,0 });
			res->name = "ZeroColor";
			return res;
		});



	//	FlowGraph::FlowSystem::get().register_node<ResultNode>("Material");
	//	FlowGraph::FlowSystem::get().register_node<MaterialGraph>("MaterialGraph");
	FlowGraph::FlowSystem::get().register_node<MaterialFunction>("MaterialFunction");
	FlowGraph::FlowSystem::get().register_node("file", []()-> FlowGraph::graph::ptr
		{
			auto f = FileSystem::get().get_file("graph.flg");

			if (f)
				return Serializer::deserialize<FlowGraph::graph>(f->load_all());

			return nullptr;
		});


	auto result_code = 0;
	SetupDebug();
	EVENT("start");
	Application::create<RenderApplication>();
	EVENT("create");


	// There can be error while creating, so test
	if (Application::is_good())
	{
		EVENT("run");
		Application::get().run(); // TODO: Create task and run engine (scheduling tasks)
		Application::reset();
	}

	else
		result_code = S_FALSE;

	CounterManager::get().print();
	EVENT("end");
	CoUninitialize();
	SymCleanup(process);
	return result_code;
}

inline void Texture3DTiled::remove_tile(ivec3 pos, int level)
{
	remove_tile(get_tile(level, pos));
}
