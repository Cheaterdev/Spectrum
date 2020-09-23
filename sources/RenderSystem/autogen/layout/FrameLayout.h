#pragma once
struct FrameLayout
{
	struct CameraData
	{
		static const unsigned int ID = 0;
		static const unsigned int CB = 1;
		static const unsigned int CB_ID = 0;
		static const unsigned int SRV = 3;
		static const unsigned int SRV_ID = 1;
	};
	struct SceneData
	{
		static const unsigned int ID = 1;
		static const unsigned int CB = 1;
		static const unsigned int CB_ID = 2;
		static const unsigned int SRV = 5;
		static const unsigned int SRV_ID = 3;
	};
	template<class Processor> static void for_each(Processor& processor) {
		processor.process<CameraData,SceneData>({Render::Samplers::SamplerLinearWrapDesc,Render::Samplers::SamplerPointClampDesc,Render::Samplers::SamplerLinearClampDesc});
	}
};