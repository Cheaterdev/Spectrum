// The GBuffer mesh + amplification shaders with the debug view's replay and
// per-primitive outputs compiled in. A file-level define rather than a PSO
// permutation: the AS and MS must agree on it (payload size, PS linkage), and
// a per-stage permutation would also build the mismatched combinations.
#define DEBUG_VIEW
#include "../gbuffer/mesh_shader.hlsl"
