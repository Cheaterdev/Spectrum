#pragma once
#include "FrameLayout.h"
struct DefaultLayout: public FrameLayout
{
	struct Instance0
	{
		static const unsigned int ID = 2;
		static const unsigned int CB = 16;
		static const unsigned int CB_ID = 4;
		static const unsigned int SRV = 4;
		static const unsigned int SRV_ID = 5;
		static const unsigned int UAV = 4;
		static const unsigned int UAV_ID = 6;
	};
	struct Instance1
	{
		static const unsigned int ID = 3;
		static const unsigned int CB = 3;
		static const unsigned int CB_ID = 7;
		static const unsigned int SRV = 7;
		static const unsigned int SRV_ID = 8;
		static const unsigned int UAV = 8;
		static const unsigned int UAV_ID = 9;
	};
	struct Instance2
	{
		static const unsigned int ID = 4;
		static const unsigned int CB = 1;
		static const unsigned int CB_ID = 10;
		static const unsigned int SRV = 6;
		static const unsigned int SRV_ID = 11;
		static const unsigned int UAV = 3;
		static const unsigned int UAV_ID = 12;
	};
	struct Raytracing
	{
		static const unsigned int ID = 5;
		static const unsigned int CB = 1;
		static const unsigned int CB_ID = 13;
		static const unsigned int SRV = 2;
		static const unsigned int SRV_ID = 14;
	};
	struct MaterialData
	{
		static const unsigned int ID = 6;
		static const unsigned int CB = 2;
		static const unsigned int CB_ID = 15;
	};
	template<class Processor> static void for_each(Processor& processor) {
		processor.process<CameraData,SceneData,Instance0,Instance1,Instance2,Raytracing,MaterialData>({Render::Samplers::SamplerLinearWrapDesc,Render::Samplers::SamplerPointClampDesc,Render::Samplers::SamplerLinearClampDesc});
	}
};
