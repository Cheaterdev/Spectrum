#pragma once
struct FrameLayout
{
	struct CameraData
	{
		static const unsigned int ID = 0;
		static const unsigned int CB = 1;
		static const unsigned int CB_ID = 0;
		static const unsigned int SRV = 0;
		static const unsigned int SRV_ID = -1;
		static const unsigned int UAV = 0;
		static const unsigned int UAV_ID = -1;
		static const unsigned int SMP = 0;
		static const unsigned int SMP_ID = -1;
	};
	struct MaterialData
	{
		static const unsigned int ID = 1;
		static const unsigned int CB = 2;
		static const unsigned int CB_ID = 1;
		static const unsigned int SRV = 0;
		static const unsigned int SRV_ID = -1;
		static const unsigned int UAV = 0;
		static const unsigned int UAV_ID = -1;
		static const unsigned int SMP = 0;
		static const unsigned int SMP_ID = -1;
	};
	template<class Processor> static void for_each(Processor& processor) {
		processor.process<CameraData,MaterialData>({Render::Samplers::SamplerLinearWrapDesc,Render::Samplers::SamplerPointClampDesc,Render::Samplers::SamplerLinearClampDesc});
	}
};
