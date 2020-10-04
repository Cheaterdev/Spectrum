
class camera;
class debug_drawer;
class vertex_transform;
class GBuffer;
namespace materials
{
    class material;
	class universal_material;
    class Pipeline;
}

namespace Render
{
	

	struct EyeInfo:public Holder
	{
		Render::Texture::ptr color_buffer;
		quat dir;
		vec3 offset;
		float fov = -1;
		camera* cam;
        GBuffer* g_buffer;
	};

	struct  OVRContext
	{
		std::vector<EyeInfo> eyes;

	};


    class context
    {
        public:

            Render::CommandList::ptr& command_list;
			Render::CommandList::ptr command_list_label;
			std::shared_ptr<OVRContext>& ovr_context;

			SingleThreadExecutorBatched* labeled;
			Batch* data;
			debug_drawer* drawer;
            sizer_long ui_clipping;
			sizer_long scissors;
            vec2 offset;
            float delta_time;
            vec2 window_size;
            float scale = 1;
            float4 color = float4(1, 1, 1, 1);
			
			std::function<void(Render::context &c)> commit_scissor()
			{
				scissors = ui_clipping;
				auto list = command_list;
				auto clip = ui_clipping;
				return [list, clip](Render::context &c) {list->get_graphics().set_scissors(clip); };
			}

			
            context(Render::CommandList::ptr& list, std::shared_ptr<OVRContext>& ovr_context): command_list(list), command_list_label(command_list_label), ovr_context(ovr_context)
            {
                drawer = nullptr;
//                cam = nullptr;
                delta_time = 0;
				

            }
    };

    class renderable
    {
        public:
            renderable() = default;
            virtual  ~renderable() = default;

            //    virtual void draw(context&) = 0;
    };



}
using namespace Render;
struct MeshRenderContext;
class MaterialProvider
{
    public:
        virtual ~MaterialProvider() {};


        virtual void use_material(size_t i, std::shared_ptr<MeshRenderContext>& context) = 0;
};
enum class RENDER_TYPE
{
    PIXEL, VOXEL
};



enum class MESH_TYPE :int {
	STATIC = 1,
	DYNAMIC = 2,
	FREEZED = 4,
	ALL = 7

};


inline bool operator &(MESH_TYPE a, MESH_TYPE b)
{
	return static_cast<int>(a) & static_cast<int>(b);
}

struct MeshRenderContext
{
    private:
        //  std::vector<std::shared_ptr<materials::material>> materials;
        Render::PipelineState::ptr current_state;
        //  Render::PipelineStateDesc current_state_desc;
    public:
        using ptr = s_ptr<MeshRenderContext>;
        int draw_count = 0;

        std::shared_ptr<materials::Pipeline> overrided_pipeline;

        MaterialProvider* mat_provider = nullptr;

        Render::PipelineStateDesc pipeline;
		std::shared_ptr<Render::OVRContext> eye_context;
	
        Render::CommandList::ptr list;
     
        camera* cam = nullptr;
        RENDER_TYPE render_type = RENDER_TYPE::PIXEL;
		MESH_TYPE render_mesh = MESH_TYPE::ALL;
       TaskPriority priority = TaskPriority::NORMAL;
        float delta_time = 0;
        size_t current_time = 0;
       // Render::Handle set_4_table;
		vec2 screen_subsample = {0,0};
        GBuffer* g_buffer = nullptr;
        RT::Slot::GBuffer::Compiled gbuffer_compiled;

        Render::Texture::ptr target_tex;
      //  Render::HandleTable voxel_target;

    
		vec3 sky_dir;
        MeshRenderContext()
        {
            pipeline.root_signature = get_Signature(Layouts::DefaultLayout);
  }
        void begin()
        {
            list->get_graphics().set_signature(pipeline.root_signature);
            list->get_compute().set_signature(pipeline.root_signature);
            draw_count = 0;
        }
        void flush_pipeline()
        {
            if (!current_state || !(current_state->desc == pipeline))
            {
                current_state = Render::PipelineStateCache::get_cache(pipeline);

              
                assert(pipeline == current_state->desc);
            }

            list->get_graphics().set_pipeline(current_state);

            if (current_state->desc.topology == D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH)
            {
                list->get_graphics().set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
			}
            else
            {
                list->get_graphics().set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            }
        
        }

        void draw_indexed(int count, int offset, int v_offset)
        {
            flush_pipeline();

       
            list->get_graphics().draw_indexed(count, offset, v_offset);

            draw_count++;
        }
 
};



class RenderTargetTable
{
        Render::HandleTableLight rtv_table;
    
        std::vector<DXGI_FORMAT>formats;
        DXGI_FORMAT depth_format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
        //  Render::Handle depth_handle;

        std::vector<Render::TextureView> textures;
        Render::TextureView depth_texture;

        std::vector<Render::Viewport> vps;
        std::vector<sizer_long> scissors;

        void on_init(ivec2 size)
        {
            vps.resize(1);
            vps[0].MinDepth = 0.0f;
            vps[0].MaxDepth = 1.0f;
            scissors.resize(1);
            vps[0].Width = static_cast<float>(size.x);
            vps[0].Height = static_cast<float>(size.y);
            vps[0].TopLeftX = 0;
            vps[0].TopLeftY = 0;
            scissors[0] = { 0, 0, size.x, size.y };
        }
    public:
		Render::HandleTableLight dsv_table;

        void clear_depth(MeshRenderContext::ptr& context, float value = 1)
        {
			if (depth_texture)
                context->list->clear_depth(dsv_table[0], value);
        }
        void clear_stencil(Render::GraphicsContext& list, UINT8 stencil = 0)
        {
			if (depth_texture)
                list.get_base().clear_stencil(dsv_table[0], stencil);
        }

        void clear_depth(Render::GraphicsContext& list, float value = 1)
        {

            if (depth_texture)
                list.get_base().clear_depth(dsv_table[0], value);
        }
        RenderTargetTable() {}


        void set_window(Render::GraphicsContext& context)
        {
            context.set_viewports(vps);
            context.set_scissors(scissors[0]);
        }

        RenderTargetTable(Render::GraphicsContext& graphics, std::initializer_list<Render::TextureView> list, Render::TextureView depth)
        {
           rtv_table = graphics.place_rtv(list.size());
            UINT i = 0;

            for (auto e : list)
            {
                formats.emplace_back(e.get_desc().Format);
                textures.emplace_back(e);
                e.place_rtv(rtv_table[i++]);
            }

            if (depth)
            {
				dsv_table = graphics.place_dsv(1);


                depth_texture = depth;
                depth_format = to_dsv(depth.get_desc().Format);
                depth.place_dsv(dsv_table[0]);
                on_init(depth.get_size());
                //      depth_handle = depth->get_ds();
            }

            else
            {
                on_init(textures[0].get_size());
            }
        }

        void set(MeshRenderContext::ptr& context,bool clear_color = false, bool clear_depth = false)
        {
 
            context->pipeline.rtv.rtv_formats = formats;
            context->pipeline.rtv.ds_format = depth_format;
            set(context->list->get_graphics(), clear_color, clear_depth);
        }

        void set(Render::GraphicsContext& graphics,bool clear_color = false, bool clear_depth = false)
		{

            auto& list = graphics.get_base();

            graphics.set_rtv(rtv_table, dsv_table.valid()?dsv_table[0]:Render::Handle());

			if (clear_color)
			{
				for (auto& tex : textures)
					list.clear_rtv(tex.get_rtv());
			}


			if (clear_depth && depth_texture)
			{
				if (depth_texture)
					list.clear_depth(dsv_table[0], 1);
			}
        }
};


class GBuffer
{
public:
	Render::TextureView albedo;
	Render::TextureView normals;
	Render::TextureView depth;
	Render::TextureView specular;
	Render::TextureView speed;


	Render::TextureView quality;
	Render::TextureView depth_mips;
	Render::TextureView depth_prev_mips;

	RenderTargetTable rtv_table;


	struct {
		Render::TextureView hiZ_depth, hiZ_depth_uav;
		RenderTargetTable hiZ_table;
	}HalfBuffer;


	void SetTable(Table::GBuffer& table)
	{
		table.GetAlbedo() = albedo.get_srv();
		table.GetNormals() = normals.get_srv();
		table.GetSpecular() = specular.get_srv();
		table.GetDepth() = depth_mips.get_srv();
        table.GetMotion() = speed.get_srv();
	}


};



class GBufferViewDesc
{
public:
	ResourceHandler* albedo = nullptr;
	ResourceHandler* normals = nullptr;
	ResourceHandler* depth = nullptr;
	ResourceHandler* specular = nullptr;
	ResourceHandler* motion = nullptr;

	ResourceHandler* quality = nullptr;
	ResourceHandler* depth_mips = nullptr;

	ResourceHandler* depth_prev = nullptr;

public:

	void create(ivec2 size, TaskBuilder& builder)
	{
		albedo = builder.create_texture("GBuffer_Albedo", size, 1, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, ResourceFlags::RenderTarget);
		normals = builder.create_texture("GBuffer_Normals", size, 1, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, ResourceFlags::RenderTarget);
		depth = builder.create_texture("GBuffer_Depth", size, 1, DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, ResourceFlags::DepthStencil);
		specular = builder.create_texture("GBuffer_Specular", size, 1, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, ResourceFlags::RenderTarget);
		motion = builder.create_texture("GBuffer_Speed", size, 1, DXGI_FORMAT::DXGI_FORMAT_R16G16_FLOAT, ResourceFlags::RenderTarget);


        depth_mips = builder.create_texture("GBuffer_DepthMips", size, 1, DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, ResourceFlags::RenderTarget | ResourceFlags::Static);
        depth_prev = builder.create_texture("GBuffer_DepthPrev", size, 1, DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, ResourceFlags::PixelRead| ResourceFlags::Static);
	}
	void create_quality(ivec2 size, TaskBuilder& builder)
	{
		quality = builder.create_texture("GBuffer_Quality", size, 1, DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT_S8X24_UINT, ResourceFlags::DepthStencil);
	}
   
	void create_mips(ivec2 size, TaskBuilder& builder)
	{
       
	}


	auto create_temp_color(ivec2 size, TaskBuilder& builder)
	{
		return builder.create_texture("GBuffer_TempColor", size, 1, DXGI_FORMAT::DXGI_FORMAT_R8G8_UNORM, ResourceFlags::RenderTarget);
	}

    void need(TaskBuilder& builder, bool need_quality = false, bool need_mips = false)
	{
		albedo = builder.need_texture("GBuffer_Albedo", ResourceFlags::PixelRead);
		normals = builder.need_texture("GBuffer_Normals", ResourceFlags::PixelRead);
		depth = builder.need_texture("GBuffer_Depth", ResourceFlags::PixelRead);
		specular = builder.need_texture("GBuffer_Specular", ResourceFlags::PixelRead);
		motion = builder.need_texture("GBuffer_Speed", ResourceFlags::PixelRead);

        depth_prev = builder.need_texture("GBuffer_DepthPrev", ResourceFlags::PixelRead);
        if(need_quality) quality = builder.need_texture("GBuffer_Quality", ResourceFlags::DSRead);
		depth_mips = builder.need_texture("GBuffer_DepthMips", ResourceFlags::None);

	}



 
	GBuffer actualize(FrameContext& context)
	{
		GBuffer result;

		result.albedo = context.get_texture(albedo);
		result.normals = context.get_texture(normals);
		result.depth = context.get_texture(depth);
		result.specular = context.get_texture(specular);
		result.speed = context.get_texture(motion);


		result.depth_prev_mips = context.get_texture(depth_prev);

        if(quality)	result.quality = context.get_texture(quality);
		if (depth_mips)	result.depth_mips = context.get_texture(depth_mips);

		return result;
	}
};
