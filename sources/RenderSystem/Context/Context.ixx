export module Graphics:Context;
import Core;
import HAL;


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
		MeshRenderContext()
		{
		
		}
		void begin()
		{
			draw_count = 0;
		}
		

	};

	class GBuffer
	{
	public:
		HAL::TextureView albedo;
		HAL::TextureView normals;
		HAL::TextureView depth;
		HAL::TextureView specular;
		HAL::TextureView speed;


		HAL::TextureView quality;
		HAL::TextureView depth_mips;
		HAL::TextureView depth_prev_mips;

		RT::GBuffer::Compiled compiled;

	//	RenderTargetTable rtv_table;


		struct {
			HAL::TextureView hiZ_depth, hiZ_depth_uav;
			//RenderTargetTable hiZ_table;
				RT::DepthOnly::Compiled compiled;

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
			builder.create(GBuffer_Albedo, { ivec3(size,0), HAL::Format::R8G8B8A8_UNORM,1,1 }, ResourceFlags::RenderTarget);
			builder.create(GBuffer_Normals, { ivec3(size,0), HAL::Format::R8G8B8A8_UNORM,1,1 }, ResourceFlags::RenderTarget);
			builder.create(GBuffer_Depth, { ivec3(size,0), HAL::Format::R32_TYPELESS,1,1 }, ResourceFlags::DepthStencil);
			builder.create(GBuffer_Specular, { ivec3(size,0), HAL::Format::R8G8B8A8_UNORM,1,1 }, ResourceFlags::RenderTarget);
			builder.create(GBuffer_Speed, { ivec3(size,0), HAL::Format::R16G16_FLOAT,1, 1 }, ResourceFlags::RenderTarget);


			builder.create(GBuffer_DepthMips, { ivec3(size,0), HAL::Format::R32_TYPELESS,1,1 }, ResourceFlags::RenderTarget | ResourceFlags::Static);
			builder.create(GBuffer_DepthPrev, { ivec3(size,0), HAL::Format::R32_TYPELESS,1,1 }, ResourceFlags::Static);
		}

		void create_quality(ivec2 size, TaskBuilder& builder)
		{
			builder.create(GBuffer_Quality, { ivec3(size,0), HAL::Format::D24_UNORM_S8_UINT,1,1 }, ResourceFlags::DepthStencil);
		}

		void create_mips(ivec2 size, TaskBuilder& builder)
		{

		}


		auto create_temp_color(ivec2 size, TaskBuilder& builder)
		{
			return builder.create(GBuffer_TempColor, { ivec3(size,0), HAL::Format::R8G8_UNORM,1,1 }, ResourceFlags::RenderTarget);
		}

		void need(TaskBuilder& builder, bool need_quality = false, bool need_mips = false)
		{
			builder.need(GBuffer_Albedo, ResourceFlags::PixelRead|ResourceFlags::ComputeRead);
			builder.need(GBuffer_Normals, ResourceFlags::PixelRead|ResourceFlags::ComputeRead);
			builder.need(GBuffer_Depth, ResourceFlags::PixelRead|ResourceFlags::ComputeRead);
			builder.need(GBuffer_Specular, ResourceFlags::PixelRead|ResourceFlags::ComputeRead);
			builder.need(GBuffer_Speed, ResourceFlags::PixelRead|ResourceFlags::ComputeRead);

			builder.need(GBuffer_DepthPrev, ResourceFlags::PixelRead|ResourceFlags::ComputeRead);
			if (need_quality) builder.need(GBuffer_Quality, ResourceFlags::DSRead);
			builder.need(GBuffer_DepthMips, ResourceFlags::None);

		}




		GBuffer actualize(FrameContext& context)
		{
			GBuffer result;

			result.albedo = *GBuffer_Albedo;
			result.normals = *GBuffer_Normals;
			result.depth = *GBuffer_Depth;
			result.specular = *GBuffer_Specular;
			result.speed = *GBuffer_Speed;


			result.depth_prev_mips = *GBuffer_DepthPrev;

			if (GBuffer_Quality)	result.quality = *GBuffer_Quality;
			if (GBuffer_DepthMips)	result.depth_mips = *GBuffer_DepthMips;


				
				


			return result;
		}
	};
}