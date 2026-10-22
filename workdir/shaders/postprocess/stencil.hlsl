#include "../common/common.hlsl"
#include "../common/common_2d.hlsl"

#ifdef BUILD_FUNC_PS_COLOR
#include "../autogen/Color.h"
#include "gizmo_pick.hlsl"

float4 PS_COLOR(vertex_output i) :SV_TARGET0
{
	gizmo_pick(i.pos);
	return GetColor().GetColor();
}
#endif

#ifdef BUILD_FUNC_PS_MASK
#include "../autogen/MaskDepth.h"

// Selected part's silhouette: alpha = 0.5 * visibility (gizmo draws add 1 on
// top). The mesh is rasterized without a depth buffer, so occlusion is tested
// here against the scene depth (reversed-Z, nearer = larger). That depth was
// rendered with the jittered projection and this mesh with the unjittered
// one, so (1) the depth is read back shifted by the jitter, onto the
// unjittered grid, and (2) visibility is a soft, tent-weighted fraction over a
// 5x5 of render texels, not a per-pixel yes/no: a hard compare crawls along
// every occluder edge as the jitter moves it (the outline resolve then
// accumulates what is left over time). A tap occludes only if it belongs to ANOTHER
// object whose surface is clearly in front of the fragment (relative
// tolerance); a depth compare alone also fires on the mesh's own steep
// surfaces and punches holes in the mask, which show up as inner outlines.
float4 PS_MASK(vertex_output i) : SV_TARGET0
{
	MaskDepth mask = GetMaskDepth();
	Texture2D<float> depth = mask.GetScene_depth();
	uint2 dims;
	depth.GetDimensions(dims.x, dims.y);
	int2 p = int2((i.pos.xy * mask.GetInv_target_size() + mask.GetJitter_uv()) * float2(dims));

	// Tent weights (3 - |x|)(3 - |y|) over -2..2 sum to 81.
	float visible = 0;
	[unroll] for (int y = -2; y <= 2; y++)
	[unroll] for (int x = -2; x <= 2; x++)
	{
		int2 q = clamp(p + int2(x, y), 0, int2(dims) - 1);
		float w = (3 - abs(x)) * (3 - abs(y));
		// The mesh's own texels and the sky never occlude it; only another
		// object can, and only when it is in front.
		uint id = mask.GetObject_ids().Load(int3(q, 0));
		bool seen = id == mask.GetSelected_id() || id == 0;
		if (!seen)
			seen = i.pos.z >= depth.Load(int3(q, 0)) * 0.98;
		visible += seen ? w : 0;
	}

	if (visible == 0)
		discard;
	return float4(0, 0, 0, 0.5 * visible / 81.0);
}
#endif
