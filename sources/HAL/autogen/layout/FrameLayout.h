#pragma once
struct FrameLayout
{
	struct CameraData
	{
		static const unsigned int ID = 0;
		static const unsigned int CB = 34;
		static const unsigned int CB_ID = 0;
		static const unsigned int SRV = 3;
		static const unsigned int SRV_ID = 2;
		static inline const std::vector<UINT> tables = {0, 2};
	};
	struct SceneData
	{
		static const unsigned int ID = 1;
		static const unsigned int CB = 2;
		static const unsigned int CB_ID = 3;
		static const unsigned int SRV = 4;
		static const unsigned int SRV_ID = 5;
		static inline const std::vector<UINT> tables = {3, 5};
	};
	struct PassData
	{
		static const unsigned int ID = 2;
		static const unsigned int CB = 2;
		static const unsigned int CB_ID = 6;
		static const unsigned int SRV = 1;
		static const unsigned int SRV_ID = 8;
		static inline const std::vector<UINT> tables = {6, 8};
	};
	struct DebugInfo
	{
		static const unsigned int ID = 3;
		static const unsigned int CB = 2;
		static const unsigned int CB_ID = 9;
		static const unsigned int UAV = 1;
		static const unsigned int UAV_ID = 11;
		static inline const std::vector<UINT> tables = {9, 11};
	};
	template<class Processor> static void for_each(Processor& processor) {
		processor.template process<CameraData,SceneData,PassData,DebugInfo>({HAL::Samplers::SamplerLinearWrapDesc,HAL::Samplers::SamplerPointClampDesc,HAL::Samplers::SamplerLinearClampDesc,HAL::Samplers::SamplerAnisoBorderDesc,HAL::Samplers::SamplerPointBorderDesc});
	}
};
