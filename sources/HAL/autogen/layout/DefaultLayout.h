#pragma once
#include "FrameLayout.h"
struct DefaultLayout: public FrameLayout
{
	struct Instance0
	{
		static const unsigned int ID = 4;
		static const unsigned int CB = 16;
		static const unsigned int CB_ID = 12;
		static const unsigned int SRV = 4;
		static const unsigned int SRV_ID = 14;
		static const unsigned int UAV = 4;
		static const unsigned int UAV_ID = 15;
		static inline const std::vector<UINT> tables = {12, 14, 15};
	};
	struct Instance1
	{
		static const unsigned int ID = 5;
		static const unsigned int CB = 6;
		static const unsigned int CB_ID = 16;
		static const unsigned int SRV = 10;
		static const unsigned int SRV_ID = 18;
		static const unsigned int UAV = 8;
		static const unsigned int UAV_ID = 19;
		static inline const std::vector<UINT> tables = {16, 18, 19};
	};
	struct Instance2
	{
		static const unsigned int ID = 6;
		static const unsigned int CB = 4;
		static const unsigned int CB_ID = 20;
		static const unsigned int SRV = 6;
		static const unsigned int SRV_ID = 22;
		static const unsigned int UAV = 3;
		static const unsigned int UAV_ID = 23;
		static inline const std::vector<UINT> tables = {20, 22, 23};
	};
	struct Raytracing
	{
		static const unsigned int ID = 7;
		static const unsigned int CB = 2;
		static const unsigned int CB_ID = 24;
		static const unsigned int SRV = 2;
		static const unsigned int SRV_ID = 26;
		static inline const std::vector<UINT> tables = {24, 26};
	};
	struct MaterialData
	{
		static const unsigned int ID = 8;
		static const unsigned int CB = 2;
		static const unsigned int CB_ID = 27;
		static const unsigned int SRV = 1;
		static const unsigned int SRV_ID = 29;
		static inline const std::vector<UINT> tables = {27, 29};
	};
	template<class Processor> static void for_each(Processor& processor) {
		processor.template process<CameraData,SceneData,PassData,DebugInfo,Instance0,Instance1,Instance2,Raytracing,MaterialData>({HAL::Samplers::SamplerLinearWrapDesc,HAL::Samplers::SamplerPointClampDesc,HAL::Samplers::SamplerLinearClampDesc,HAL::Samplers::SamplerAnisoBorderDesc,HAL::Samplers::SamplerPointBorderDesc});
	}
};
