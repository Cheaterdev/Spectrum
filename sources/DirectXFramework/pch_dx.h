#pragma once
#include "pch_core.h"
#include "utils/utils_macros.h"

import d3d12;

HRESULT device_fail();
HRESULT test(HRESULT hr, std::string str = "");

#ifdef  DEBUG
#define TEST(x)\
	assert(SUCCEEDED(x))
#else
#define TEST(x)\
	([&](){\
	HRESULT hr = x; \
	if(hr==0x887a0005) device_fail(); \
	 test(hr, STRINGIZE(x)); \
if (FAILED(hr)) \
	{__debugbreak(); \
	assert(false); }\
	return hr; \
	})()

#endif //  DEBUG


namespace DX12
{
	
}
namespace Render
{
	using namespace DX12;
}




#define GEN_KEY(x, dir) static inline const KeyGenerator<&Keys::x,dir> x;


#define GEN_COMPUTE_PSO(name, ...) \
using PSOState = Render::ComputePipelineState; \
using SimplePSO = SimpleComputePSO; \
static const PSO ID = PSO::name;\
std::map<Keys, PSOState::ptr> psos = {}; \
PSOState::ptr GetPSO(KeyPair<Keys> key = KeyPair<Keys>()) {return psos[key.GetKey()];}; \
name() \
{ \
	PSOBase::shuffle_pairs<name>([&](Keys& key) \
		{ \
			auto pso = init_pso(key); \
			psos[key] = pso.create(); \
		} , ## __VA_ARGS__); \
}


#define GEN_GRAPHICS_PSO(name, ...) \
using PSOState = Render::PipelineState; \
using SimplePSO = SimpleGraphicsPSO; \
static const PSO ID = PSO::name;\
std::map<Keys, PSOState::ptr> psos = {}; \
PSOState::ptr GetPSO(KeyPair<Keys> key = KeyPair<Keys>()) {return psos[key.GetKey()];}; \
name() \
{ \
	PSOBase::shuffle_pairs<name>([&](Keys& key)\
		{\
			auto pso = init_pso(key);\
			psos[key] = pso.create();\
		} , ## __VA_ARGS__);\
}



