#pragma once
// PSO class body-injection macros for HAL and all dependent projects.
// Included by HAL/Defines.h via forced include.

#ifndef GEN_KEY
#  define GEN_KEY(x, dir)  static inline const KeyGenerator<&Keys::x, dir> x;
#endif

#ifndef GEN_COMPUTE_PSO
#  define GEN_COMPUTE_PSO(name, ...)                                                  \
       using ptr      = std::shared_ptr<name>;                                        \
       using PSOState = HAL::ComputePipelineState;                                    \
       using SimplePSO = SimpleComputePSO;                                            \
       static const PSO ID = PSO::name;                                               \
       std::map<Keys, PSOState::ptr> psos = {};                                       \
       PSOState::ptr GetPSO(KeyPair<Keys> key = KeyPair<Keys>()) {                    \
           ASSERT(psos[key.GetKey()]); return psos[key.GetKey()];                     \
       };                                                                             \
       name(HAL::Device& device,                                                      \
            std::function<void(SimplePSO&, Keys&)> modifier = nullptr)                \
       {                                                                              \
           PSOBase::shuffle_pairs<name>([&](Keys& key) {                              \
               auto pso = init_pso(key, modifier);                                    \
               psos[key] = pso.create(device);                                        \
           } , ##__VA_ARGS__);                                                        \
       }                                                                              \
       name() = default;
#endif

#ifndef GEN_GRAPHICS_PSO
#  define GEN_GRAPHICS_PSO(name, ...)                                                 \
       using ptr      = std::shared_ptr<name>;                                        \
       using PSOState = HAL::PipelineState;                                           \
       using SimplePSO = SimpleGraphicsPSO;                                           \
       static const PSO ID = PSO::name;                                               \
       std::map<Keys, PSOState::ptr> psos = {};                                       \
       PSOState::ptr GetPSO(KeyPair<Keys> key = KeyPair<Keys>()) {                    \
           ASSERT(psos[key.GetKey()]); return psos[key.GetKey()];                     \
       };                                                                             \
       name(HAL::Device& device,                                                      \
            std::function<void(SimplePSO&, Keys&)> modifier = nullptr)                \
       {                                                                              \
           PSOBase::shuffle_pairs<name>([&](Keys& key) {                              \
               auto pso = init_pso(key, modifier);                                    \
               psos[key] = pso.create(device);                                        \
           } , ##__VA_ARGS__);                                                        \
       }                                                                              \
       name() = default;
#endif

#ifndef GEN_WORKGRAPH_PSO
#  define GEN_WORKGRAPH_PSO(name, ...)                                                \
       using ptr      = std::shared_ptr<name>;                                        \
       using PSOState = HAL::StateObject;                                             \
       using SimplePSO = SimpleWorkgraphPSO;                                          \
       static const PSO ID = PSO::name;                                               \
       std::map<Keys, PSOState::ptr> psos = {};                                       \
       PSOState::ptr GetPSO(KeyPair<Keys> key = KeyPair<Keys>()) {                    \
           ASSERT(psos[key.GetKey()]); return psos[key.GetKey()];                     \
       };                                                                             \
       name(HAL::Device& device,                                                      \
            std::function<void(SimplePSO&, Keys&)> modifier = nullptr)                \
       {                                                                              \
           PSOBase::shuffle_pairs<name>([&](Keys& key) {                              \
               auto pso = init_pso(key, modifier);                                    \
               psos[key] = pso.create(device);                                        \
           } , ##__VA_ARGS__);                                                        \
       }                                                                              \
       name() = default;
#endif
