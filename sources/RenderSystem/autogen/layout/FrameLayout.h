#pragma once
struct FrameLayout
{
	struct CameraData
	{
		static const unsigned int ID = 0;
		static const unsigned int CB = 32;
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
	struct DebugInfo
	{
		static const unsigned int ID = 2;
		static const unsigned int CB = 1;
		static const unsigned int CB_ID = 4;
		static const unsigned int UAV = 1;
		static const unsigned int UAV_ID = 5;
	};
	template<class Processor> static void for_each(Processor& processor) {
		processor.process<CameraData,SceneData,DebugInfo>({DX12::Samplers::SamplerLinearWrapDesc,DX12::Samplers::SamplerPointClampDesc,DX12::Samplers::SamplerLinearClampDesc,DX12::Samplers::SamplerAnisoBorderDesc});
	}
};
