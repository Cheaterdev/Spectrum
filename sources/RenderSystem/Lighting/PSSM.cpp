module Graphics:PSSM;
import <RenderSystem.h>;

import :PSSM;
import :BRDF;
import :EngineAssets;

import HAL;


import Graphics;
using namespace FrameGraph;
float jit = 0;
import HAL;
using namespace HAL;
float3 PSSM::get_position()
{
	pos_mutex.lock();
	auto p = position;
	pos_mutex.unlock();
	return p;
}

void PSSM::set_position(float3 p)
{
	pos_mutex.lock();
	position = p;
	pos_mutex.unlock();
}

PSSM::PSSM()
{
	position = float3(200, 400, 200);
}
