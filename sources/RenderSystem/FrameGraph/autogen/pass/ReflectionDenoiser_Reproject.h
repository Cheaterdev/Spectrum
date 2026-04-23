
using namespace FrameGraph;

namespace Passes
{

class ReflectionDenoiser_Reproject 
{
public:
	struct Context
	{

		Handlers::Texture H(GBuffer_DepthPrev);


		Handlers::Texture H(GBuffer_NormalsPrev);


		Handlers::Texture H(GBuffer_Depth);


		Handlers::Texture H(GBuffer_Normals);


		Handlers::Texture H(GBuffer_Speed);


		Handlers::Texture H(VoxelReflectionNoise);


		Handlers::Texture H(ReflectionDenoiser_RadiancePrev);


		Handlers::Texture H(ReflectionDenoiser_AverageRadiance);


		Handlers::Texture H(ReflectionDenoiser_AverageRadiancePrev);


		Handlers::Texture H(ReflectionDenoiser_Variance);


		Handlers::Texture H(ReflectionDenoiser_VariancePrev);


		Handlers::Texture H(ReflectionDenoiser_SampleCount);


		Handlers::Texture H(ReflectionDenoiser_SampleCountPrev);


		Handlers::Texture H(ReflectionDenoiser_ReprojectedRadiance);


		Handlers::Texture H(BlueNoise);

	};


	static inline const wchar_t* Name = L"ReflectionDenoiser_Reproject";

//	static constexpr PassID ID = PassID::ReflectionDenoiser_Reproject;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}