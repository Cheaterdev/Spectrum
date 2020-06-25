#include "../Renderer/Signature.h"
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

        MaterialProvider* mat_provider;

        Render::PipelineStateDesc pipeline;
		std::shared_ptr<Render::OVRContext> eye_context;
	
        Render::CommandList::ptr list;
        std::shared_ptr<vertex_transform> transformer;
        camera* cam;
        RENDER_TYPE render_type = RENDER_TYPE::PIXEL;
		MESH_TYPE render_mesh = MESH_TYPE::ALL;
       TaskPriority priority = TaskPriority::NORMAL;
        float delta_time = 0;
        size_t current_time = 0;
       // Render::Handle set_4_table;
		vec2 screen_subsample = {0,0};
        GBuffer* g_buffer;
        /*    void set_materials(std::vector<std::shared_ptr<materials::material>>& m)
            {
                if (!override_material)
                    materials = m;
            }*/

        Render::Texture::ptr target_tex;
        Render::HandleTable voxel_target;
        vec3 voxel_min;
        vec3 voxel_size;
		ivec3 voxel_target_size;
		ivec3 voxel_tiles_count;
		ivec3 voxels_per_tile;
		vec3 sky_dir;
  
        void begin()
        {
            list->get_graphics().set_signature(pipeline.root_signature);
            draw_count = 0;
    //        set_4_table.cpu.ptr = 0;
    //        set_4_table.gpu.ptr = 0;

         //   if (voxel_target.valid())
          //      set_uav(voxel_target);
        }
	
        void flush_pipeline()
        {
            if (!current_state || !(current_state->desc == pipeline))
            {
                current_state = Render::PipelineStateCache::get_cache(pipeline);//new Render::PipelineState(pipeline));
            }

            list->get_graphics().set_pipeline(current_state);
        }

        void draw_indexed(int count, int offset, int v_offset)
        {
            flush_pipeline();

       
            list->get_graphics().draw_indexed(count, offset, v_offset);

            draw_count++;
        }
        /*
 
        void set_nodes_buffer(Render::FrameResource& resource)
        {
			GPUMeshSignature<Signature> signature(&list->get_graphics());

          //  list->get_graphics().set_srv(8, resource);
        }
  
        void set_mesh_const_data(unsigned int node_index)
        {
         //   list->get_graphics().set_constants(2, node_index);
			GPUMeshSignature<Signature> signature(&list->get_graphics());

        }


        void set_material_const_buffer(Render::GPUBuffer::ptr buffer)
        {
  //          list->get_graphics().set_const_buffer(0, buffer);

			GPUMeshSignature<Signature> signature(&list->get_graphics());
			signature.mat_const = buffer;
        }

        void set_material_tess_buffer(Render::GPUBuffer::ptr buffer)
        {
       //     list->get_graphics().set_const_buffer(1, buffer);
			GPUMeshSignature<Signature> signature(&list->get_graphics());
			signature.mat_const_tess = buffer;
        }
        void set_material_textures(std::vector<Render::Handle> table)
        {
			GPUMeshSignature<Signature> signature(&list->get_graphics());
       //     list->get_graphics().get_desc_manager().set(2,0, table);
			signature.mat_textures = table;
		}

        void set_material_unordered(Render::HandleTable& table)
        {
			GPUMeshSignature<Signature> signature(&list->get_graphics());
			//    list->get_graphics().set_dynamic(6,0, table);
	

        }
        void set_frame_data(Render::FrameResource& resource)
        {
			GPUMeshSignature<Signature> signature(&list->get_graphics());
			// list->get_graphics().set_const_buffer(6, resource);
			signature.camera_info = resource;
        }

        void set_uav(const Render::HandleTable& table)
		{
			GPUMeshSignature<Signature> signature(&list->get_graphics());
			//             list->get_graphics().set_dynamic(9,0, table);
			signature.voxel_output[0] = table;
        }
        */
 
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
                context->list->get_native_list()->ClearDepthStencilView(dsv_table[0].cpu, D3D12_CLEAR_FLAG_DEPTH, value, 0, 0, nullptr);
        }
        void clear_stencil(Render::GraphicsContext& list, UINT8 stencil = 0)
        {
            if (depth_texture)
                list.get_native_list()->ClearDepthStencilView(dsv_table[0].cpu,  D3D12_CLEAR_FLAG_STENCIL, 0, stencil, 0, nullptr);
        }

        void clear_depth(Render::GraphicsContext& list, float value = 1)
        {
            if (depth_texture)
                list.get_native_list()->ClearDepthStencilView(dsv_table[0].cpu, D3D12_CLEAR_FLAG_DEPTH, value, 0, 0, nullptr);
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
      /*
            for (auto& tex : textures)
			{
				list.transition(tex.resource, Render::ResourceState::RENDER_TARGET);
			}

            list.transition(depth_texture.resource, Render::ResourceState::DEPTH_WRITE);
            */

            if (clear_color)
            {
                for (auto& tex : textures)
                    list.clear_rtv(tex.get_rtv());
            }


            if (clear_depth&& depth_texture)
            {
				if (depth_texture)
                    list.get_native_list()->ClearDepthStencilView(dsv_table[0].cpu, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
            }

            graphics.set_rtv(rtv_table, dsv_table.valid()?dsv_table[0]:Render::Handle());
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

	RenderTargetTable rtv_table;


    struct {
		Render::TextureView hiZ_depth, hiZ_depth_uav;
		RenderTargetTable hiZ_table;
    }HalfBuffer;
};

