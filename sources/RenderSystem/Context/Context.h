#pragma once

import Scheduler;
import Holdable;
import Device;
import PipelineState;
#include "FrameGraph/FrameGraph.h"
#include "helper.h"
import Executors;
import Autogen;
import Quaternion;
class camera;
class debug_drawer;
class vertex_transform;
class GBuffer;

using namespace FrameGraph;

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
            vec2 offset = {0,0};
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


        SlotContext* slot_context = nullptr;
        Slots::Voxelization::Compiled voxelization_compiled;
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
           rtv_table = graphics.place_rtv((UINT)list.size());
            UINT i = 0;

            for (auto e : list)
            {
                formats.emplace_back(e.get_desc().Format);
                textures.emplace_back(e);

                rtv_table[i++].place(e.renderTarget);
             //   e.place_rtv(rtv_table[i++]);
            }

            if (depth)
            {
				dsv_table = graphics.place_dsv(1);


                depth_texture = depth;
                depth_format = to_dsv(depth.get_desc().Format);
              //  depth.place_dsv(dsv_table[0]);
                dsv_table[0].place(depth.depthStencil);
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
					list.clear_rtv(tex.renderTarget);
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
		table.GetAlbedo() = albedo.texture2D;
		table.GetNormals() = normals.texture2D;
		table.GetSpecular() = specular.texture2D;
		table.GetDepth() = depth_mips.texture2D;
        table.GetMotion() = speed.texture2D;
	}


};


class GBufferViewDesc
{
public:
    Handlers::Texture H(GBuffer_Albedo);
    Handlers::Texture H(GBuffer_Normals);
    Handlers::Texture H(GBuffer_Depth);
    Handlers::Texture H(GBuffer_Specular);
    Handlers::Texture H(GBuffer_Speed);
    Handlers::Texture H(GBuffer_DepthMips);
    Handlers::Texture H(GBuffer_DepthPrev);

    Handlers::Texture H(GBuffer_Quality);
    Handlers::Texture H(GBuffer_TempColor);


public:

	void create(ivec2 size, TaskBuilder& builder)
	{
		builder.create(GBuffer_Albedo, { ivec3(size,1), DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM,1,1}, ResourceFlags::RenderTarget);
	 builder.create(GBuffer_Normals, { ivec3(size,1), DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM,1,1}, ResourceFlags::RenderTarget);
		 builder.create(GBuffer_Depth, { ivec3(size,1), DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS,1,1}, ResourceFlags::DepthStencil);
		builder.create(GBuffer_Specular, { ivec3(size,1), DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM,1,1}, ResourceFlags::RenderTarget);
		 builder.create(GBuffer_Speed, { ivec3(size,1), DXGI_FORMAT::DXGI_FORMAT_R16G16_FLOAT,1, 1}, ResourceFlags::RenderTarget);


       builder.create(GBuffer_DepthMips, { ivec3(size,1), DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS,1,1}, ResourceFlags::RenderTarget | ResourceFlags::Static);
        builder.create(GBuffer_DepthPrev, { ivec3(size,1), DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS,1,1}, ResourceFlags::RenderTarget | ResourceFlags::Static);
	}
	
	void create_quality(ivec2 size, TaskBuilder& builder)
	{
		 builder.create(GBuffer_Quality, { ivec3(size,1), DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT,1,1}, ResourceFlags::DepthStencil);
	}
   
	void create_mips(ivec2 size, TaskBuilder& builder)
	{
       
	}


	auto create_temp_color(ivec2 size, TaskBuilder& builder)
	{
		return builder.create(GBuffer_TempColor, { ivec3(size,1), DXGI_FORMAT::DXGI_FORMAT_R8G8_UNORM,1,1}, ResourceFlags::RenderTarget);
	}

    void need(TaskBuilder& builder, bool need_quality = false, bool need_mips = false)
	{
		builder.need(GBuffer_Albedo, ResourceFlags::PixelRead);
		 builder.need(GBuffer_Normals,ResourceFlags::PixelRead);
		builder.need(GBuffer_Depth, ResourceFlags::PixelRead);
		 builder.need(GBuffer_Specular, ResourceFlags::PixelRead);
		 builder.need(GBuffer_Speed, ResourceFlags::PixelRead);

      builder.need(GBuffer_DepthPrev, ResourceFlags::PixelRead);
        if(need_quality) builder.need(GBuffer_Quality, ResourceFlags::DSRead);
		 builder.need(GBuffer_DepthMips, ResourceFlags::None);

	}



 
	GBuffer actualize(FrameContext& context)
	{
		GBuffer result;

        result.albedo = *GBuffer_Albedo;
		result.normals = *GBuffer_Normals;
		result.depth = *GBuffer_Depth;
		result.specular = *GBuffer_Specular;
		result.speed =  *GBuffer_Speed;


		result.depth_prev_mips = *GBuffer_DepthPrev;

        if(GBuffer_Quality)	result.quality = *GBuffer_Quality;
		if (GBuffer_DepthMips)	result.depth_mips = *GBuffer_DepthMips;

		return result;
	}
};
