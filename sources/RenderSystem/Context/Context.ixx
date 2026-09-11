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

	template <class T>
	 concept Has_GBuffer_Depth=	  requires(T& context)
	 {
		 context.GBuffer_Depth;
	 };

	template <class T>
	 concept Has_GBuffer_DepthPrev=	  requires(T& context)
	 {
		 context.GBuffer_DepthPrev;
	 };


	 template <class T>
	 concept Has_GBuffer_DepthMips =	  requires(T& context)
	 {
		 context.GBuffer_DepthMips;
	 };

	 	 template <class T>
	 concept Has_GBuffer_Quality =	  requires(T& context)
	 {
		 context.GBuffer_Quality;
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
	

					template<class T>
		static GBuffer actualize(T& context)
		{
			GBuffer result;

			result.albedo = *context.GBuffer_Albedo;
			result.normals = *context.GBuffer_Normals;
			result.specular = *context.GBuffer_Specular;
			result.speed = *context.GBuffer_Speed;

			// GBuffer_Depth/GBuffer_DepthPrev are read directly (not through
			// SetTable(), which only ever touches Albedo/Normals/Specular/
			// Speed/DepthMips -- see GBuffer::SetTable, Context2.cpp) by the
			// two producer passes (Scene/AssetGBuffer, for their own RTV
			// compile) and by RTXShadow alone among consumers. Every other
			// consumer only ever calls actualize()+SetTable(), so declaring
			// [Always=Read] on these two per-pass would need() a resource the
			// pass never reads -- conditional (if constexpr, like Quality/
			// DepthMips below) so those passes don't need to declare the
			// field at all, not just leave it unbound.
			if constexpr(Has_GBuffer_Depth<T>)     if (context.GBuffer_Depth)     result.depth = *context.GBuffer_Depth;
			 if constexpr(Has_GBuffer_DepthPrev<T>) if (context.GBuffer_DepthPrev) result.depth_prev_mips = *context.GBuffer_DepthPrev;

			 if constexpr(Has_GBuffer_Quality<T>) if (context.GBuffer_Quality)	result.quality = *context.GBuffer_Quality;
			 if constexpr(Has_GBuffer_DepthMips<T>) if (context.GBuffer_DepthMips)	result.depth_mips = *context.GBuffer_DepthMips;

			
				
				


			return result;
		}
	};
}