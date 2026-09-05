// NVIDIA NRD (Real-time Denoisers) — REBLUR/RELAX/SIGMA. Statically linked
// (nrd.lib), unlike streamline's manual GetProcAddress hooking: NRD makes no
// GAPI calls itself (GetComputeDispatches() returns a list of {shader,
// constants, resource bindings} for the engine to execute), so there is no
// device/swapchain proxying concern that would call for the same caution.
//
// Header-unit re-export, same shape as the streamline/assimp/magic_enum
// wrappers.
export module nrd;

export import "_nrd.h";
