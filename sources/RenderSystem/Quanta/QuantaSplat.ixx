export module Graphics:QuantaSplat;

import Core;

// The debug pass that draws Quanta's LOD cut as points (quanta.prism, QuantaSplat.cpp). Off by default; the
// toggle is "Quanta splat" under "Quanta" in the Properties panel.

export namespace Quanta
{
	bool splat_enabled();
	void set_splat_enabled(bool enabled);

	// "Quanta draw": the main view's GBuffer draws read Quanta's clusters instead of the mesh's own meshlets
	// (the QUANTA permutations, gbuffer/mesh_shader.hlsl). Off by default; needs the cluster LOD walk, and a
	// mesh without Quanta data is not drawn while it is on.
	bool draw_enabled();
	void set_draw_enabled(bool enabled);

	// With "Quanta draw" on, the shadow cascades draw the cut too: it stops refining at this many texels of the
	// cascade's map ("Shadow LOD error (px)").
	float shadow_lod_pixels();
}
