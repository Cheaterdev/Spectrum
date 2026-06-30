export module Graphics:Context;


import Core;
import HAL;
import :Texture;

import FrameGraph;

import :Camera;

class GBuffer;

using namespace FrameGraph;
export namespace materials
{
	//class material;
//class universal_material;
	class Pipeline;
}
export{
	namespace Graphics
	{


		struct EyeInfo :public Holder
		{
			HAL::Texture::ptr color_buffer;
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


		class renderable
		{
		public:
			renderable() = default;
			virtual  ~renderable() = default;

			//    virtual void draw(context&) = 0;
		};



	}
	using namespace HAL;
	struct MeshRenderContext;

	enum class RENDER_TYPE
	{
		PIXEL, VOXEL, DEPTH
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
		HAL::PipelineState::ptr current_state;
		//  HAL::PipelineStateDesc current_state_desc;
	public:
		using ptr = s_ptr<MeshRenderContext>;
		int draw_count = 0;

		std::shared_ptr<Graphics::OVRContext> eye_context;

		HAL::CommandList::ptr list;

		camera* cam = nullptr;
		RENDER_TYPE render_type = RENDER_TYPE::PIXEL;
		MESH_TYPE render_mesh = MESH_TYPE::ALL;
		TaskPriority priority = TaskPriority::NORMAL;
		float delta_time = 0;
		size_t current_time = 0;
		// HAL::Handle set_4_table;
		vec2 screen_subsample = { 0,0 };
		GBuffer* g_buffer = nullptr;
		//RT::GBuffer::Compiled gbuffer_compiled;

		HAL::Texture::ptr target_tex;


		FrameGraph::SlotContext* slot_context = nullptr;
		Slots::Voxelization::Compiled voxelization_compiled;
		vec3 sky_dir;
		MeshRenderContext();

		void begin();
		

	};

	class GBuffer
	{
	public:
		HAL::Texture2DView albedo;
		HAL::Texture2DView normals;
		HAL::Texture2DView depth;
		HAL::Texture2DView specular;
		HAL::Texture2DView speed;


		HAL::Texture2DView quality;
		HAL::Texture2DView depth_mips;
		HAL::Texture2DView depth_prev_mips;

		RT::GBuffer::Compiled compiled;

	//	RenderTargetTable rtv_table;


		struct {
			HAL::Texture2DView hiZ_depth, hiZ_depth_uav;
			//RenderTargetTable hiZ_table;
				RT::DepthOnly::Compiled compiled;

		}HalfBuffer;


		void SetTable(Table::GBuffer& table);


	};


	class GBufferViewDesc
	{
	public:
		Handlers::Texture GBuffer_Albedo = FrameGraph::ResourceID::GBuffer_Albedo;
		Handlers::Texture GBuffer_Normals = FrameGraph::ResourceID::GBuffer_Normals;
		Handlers::Texture GBuffer_Depth = FrameGraph::ResourceID::GBuffer_Depth;
		Handlers::Texture GBuffer_Specular = FrameGraph::ResourceID::GBuffer_Specular;
		Handlers::Texture GBuffer_Speed = FrameGraph::ResourceID::GBuffer_Speed;
		Handlers::Texture GBuffer_DepthMips = FrameGraph::ResourceID::GBuffer_DepthMips;
		Handlers::Texture GBuffer_DepthPrev = FrameGraph::ResourceID::GBuffer_DepthPrev;

		Handlers::Texture GBuffer_Quality = FrameGraph::ResourceID::GBuffer_Quality;
		Handlers::Texture GBuffer_TempColor = FrameGraph::ResourceID::GBuffer_TempColor;


	public:
		static void create(ivec2 size,auto &context, TaskBuilder& builder)
		{
			builder.create(context.GBuffer_Albedo, { ivec3(size,0), HAL::Format::R8G8B8A8_UNORM,1,1 }, ResourceFlags::RenderTarget);
			builder.create(context.GBuffer_Normals, { ivec3(size,0), HAL::Format::R8G8B8A8_UNORM,1,1 }, ResourceFlags::RenderTarget);
			builder.create(context.GBuffer_Depth, { ivec3(size,0), HAL::Format::R32_TYPELESS,1,1 }, ResourceFlags::DepthStencil);
			builder.create(context.GBuffer_Specular, { ivec3(size,0), HAL::Format::R8G8B8A8_UNORM,1,1 }, ResourceFlags::RenderTarget);
			builder.create(context.GBuffer_Speed, { ivec3(size,0), HAL::Format::R16G16_FLOAT,1, 1 }, ResourceFlags::RenderTarget);


			builder.create(context.GBuffer_DepthMips, { ivec3(size,0), HAL::Format::R32_TYPELESS,1,1 },ResourceFlags::UnorderedAccess | ResourceFlags::RenderTarget | ResourceFlags::Static);
			builder.create(context.GBuffer_DepthPrev, { ivec3(size,0), HAL::Format::R32_TYPELESS,1,1 }, ResourceFlags::Static);
		}

		static void create_quality(ivec2 size, auto &context, TaskBuilder& builder)
		{
			builder.create(context.GBuffer_Quality, { ivec3(size,0), HAL::Format::D24_UNORM_S8_UINT,1,1 }, ResourceFlags::DepthStencil);
		}

		static void create_mips(ivec2 size, auto &context,  TaskBuilder& builder)
		{

		}


		auto create_temp_color(ivec2 size, TaskBuilder& builder)
		{
			return builder.create(GBuffer_TempColor, { ivec3(size,0), HAL::Format::R8G8_UNORM,1,1 }, ResourceFlags::RenderTarget);
		}

		static void need(TaskBuilder& builder,auto &context, bool need_quality = false, bool need_mips = false)
		{
			builder.need(context.GBuffer_Albedo, ResourceFlags::PixelRead|ResourceFlags::ComputeRead);
			builder.need(context.GBuffer_Normals, ResourceFlags::PixelRead|ResourceFlags::ComputeRead);
			builder.need(context.GBuffer_Depth, ResourceFlags::PixelRead|ResourceFlags::ComputeRead);
			builder.need(context.GBuffer_Specular, ResourceFlags::PixelRead|ResourceFlags::ComputeRead);
			builder.need(context.GBuffer_Speed, ResourceFlags::PixelRead|ResourceFlags::ComputeRead);

			builder.need(context.GBuffer_DepthPrev, ResourceFlags::PixelRead|ResourceFlags::ComputeRead);
			if (need_quality) builder.need(context.GBuffer_Quality, ResourceFlags::DSRead);
			builder.need(context.GBuffer_DepthMips, ResourceFlags::None);

		}




		static GBuffer actualize(auto& context)
		{
			GBuffer result;

			result.albedo = *context.GBuffer_Albedo;
			result.normals = *context.GBuffer_Normals;
			result.depth = *context.GBuffer_Depth;
			result.specular = *context.GBuffer_Specular;
			result.speed = *context.GBuffer_Speed;

			result.depth_prev_mips = *context.GBuffer_DepthPrev;

			if (context.GBuffer_Quality)	result.quality = *context.GBuffer_Quality;
			if (context.GBuffer_DepthMips)	result.depth_mips = *context.GBuffer_DepthMips;


				
				


			return result;
		}
	};
}