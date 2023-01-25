
#define GEN_KEY(x, dir) static inline const KeyGenerator<&Keys::x,dir> x;


#define GEN_COMPUTE_PSO(name, ...) \
using ptr = std::shared_ptr<name>;\
using PSOState = HAL::ComputePipelineState; \
using SimplePSO = SimpleComputePSO; \
static const PSO ID = PSO::name;\
std::map<Keys, PSOState::ptr> psos = {}; \
PSOState::ptr GetPSO(KeyPair<Keys> key = KeyPair<Keys>()) {return psos[key.GetKey()];}; \
name(HAL::Device&device, std::function<void(SimplePSO&, Keys&)> modifier = nullptr) \
{ \
	PSOBase::shuffle_pairs<name>([&](Keys& key) \
		{ \
			auto pso = init_pso(key,modifier); \
			psos[key] = pso.create(device); \
		} , ## __VA_ARGS__); \
}\
name() = default;


#define GEN_GRAPHICS_PSO(name, ...) \
using ptr = std::shared_ptr<name>;\
using PSOState = HAL::PipelineState; \
using SimplePSO = SimpleGraphicsPSO; \
static const PSO ID = PSO::name;\
std::map<Keys, PSOState::ptr> psos = {}; \
PSOState::ptr GetPSO(KeyPair<Keys> key = KeyPair<Keys>()) {return psos[key.GetKey()];}; \
name(HAL::Device&device, std::function<void(SimplePSO&, Keys&)> modifier = nullptr) \
{ \
	PSOBase::shuffle_pairs<name>([&](Keys& key)\
		{\
			auto pso = init_pso(key,modifier);\
			psos[key] = pso.create(device);\
		} , ## __VA_ARGS__);\
}\
name() = default;

