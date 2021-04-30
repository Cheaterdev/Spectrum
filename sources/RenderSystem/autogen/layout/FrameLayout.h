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
		static inline const std::vector<UINT> tables = {0, 1};
	};
	struct SceneData
	{
		static const unsigned int ID = 1;
		static const unsigned int CB = 1;
		static const unsigned int CB_ID = 2;
		static const unsigned int SRV = 5;
		static const unsigned int SRV_ID = 3;
		static inline const std::vector<UINT> tables = {2, 3};
	};
	struct DebugInfo
	{
		static const unsigned int ID = 2;
		static const unsigned int CB = 1;
		static const unsigned int CB_ID = 4;
		static const unsigned int UAV = 1;
		static const unsigned int UAV_ID = 5;
		static inline const std::vector<UINT> tables = {4, 5};
	};
	template<class Processor> static void for_each(Processor& processor) {
		processor.process<CameraData,SceneData,DebugInfo>({Render::Samplers::SamplerLinearWrapDesc,Render::Samplers::SamplerPointClampDesc,Render::Samplers::SamplerLinearClampDesc,Render::Samplers::SamplerAnisoBorderDesc,Render::Samplers::SamplerPointBorderDesc});
	}
};
