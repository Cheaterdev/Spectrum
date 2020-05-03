#pragma once
#include "FrameLayout.h"
struct DefaultLayout: public FrameLayout
{
	struct Instance0
	{
		static const unsigned int ID = 3;
		static const unsigned int CB = 16;
		static const unsigned int CB_ID = 8;
		static const unsigned int SRV = 4;
		static const unsigned int SRV_ID = 9;
		static const unsigned int UAV = 4;
		static const unsigned int UAV_ID = 10;
		static const unsigned int SMP = 0;
		static const unsigned int SMP_ID = -1;
	};
	struct Instance1
	{
		static const unsigned int ID = 4;
		static const unsigned int CB = 3;
		static const unsigned int CB_ID = 12;
		static const unsigned int SRV = 2;
		static const unsigned int SRV_ID = 13;
		static const unsigned int UAV = 0;
		static const unsigned int UAV_ID = -1;
		static const unsigned int SMP = 0;
		static const unsigned int SMP_ID = -1;
	};
	struct Instance2
	{
		static const unsigned int ID = 5;
		static const unsigned int CB = 1;
		static const unsigned int CB_ID = 15;
		static const unsigned int SRV = 6;
		static const unsigned int SRV_ID = 16;
		static const unsigned int UAV = 0;
		static const unsigned int UAV_ID = -1;
		static const unsigned int SMP = 0;
		static const unsigned int SMP_ID = -1;
	};
	template<class Processor> static void for_each(Processor& processor) {
		processor.process<CameraData,SceneData,MaterialData,Instance0,Instance1,Instance2>({Render::Samplers::SamplerLinearWrapDesc,Render::Samplers::SamplerPointClampDesc,Render::Samplers::SamplerLinearClampDesc});
	}
};
