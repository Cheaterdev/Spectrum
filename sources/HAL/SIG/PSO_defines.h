
#define GEN_KEY(x, dir) static inline const KeyGenerator<&Keys::x,dir> x;


#define GEN_COMPUTE_PSO(name, ...) \
using PSOState = HAL::ComputePipelineState; \
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
using PSOState = HAL::PipelineState; \
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

