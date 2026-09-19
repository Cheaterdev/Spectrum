#include "../autogen/FrameInfo.h"
#include "../autogen/Raytracing.h"
#include "../autogen/tables/RayPayload.h"
#include "../autogen/tables/ShadowPayload.h"
#include "../autogen/tables/ColorShadowPayload.h"
#include "../autogen/rtx/ColorPass.h"
#include "../autogen/DDGIProbeTraceData.h"
#include "octahedral.hlsl"
#include "ddgi_sample.hlsl"

// v1 (see [[project-ddgi]] planning notes): one ray per radiance-atlas texel
// (dispatch dims = DDGI_AtlasWidth x DDGI_AtlasHeight), direction from
// octahedral-decoding that texel's own local UV within its probe's cell.
// Real DDGI implementations decouple ray count from texel resolution (trace
// a small fixed set of directions per probe, then resample into the
// octahedral map) -- this 1:1 mapping is a v1 simplification, not the final
// design, but it does trace and shade for real: same ColorPass hit group
// (MyClosestHitShader) IndirectRTX's own per-pixel GI ray uses, so a probe's
// radiance texel gets genuine direct-sun-lit material shading.
//
// Multi-bounce feedback: on a real hit, samples DDGI_ProbeIrradiance/
// DDGI_ProbeVisibility -- LAST frame's convolved output, see
// DDGIProbeTrace's own PassNode comment (ddgi.sig) for why reading them here
// is safe -- at the hit point/normal and adds albedo * irradiance on top of
// the direct-lit result. This is the actual multi/infinite-bounce mechanism
// (AC Shadows talk: "using values from the previous frame ... multi-bounce
// global illumination"): light keeps propagating probe-to-probe over
// successive frames instead of needing more bounces traced per frame. No pi/
// BRDF normalization on the added term yet (tuning item, not structural) and
// no round-robin budget skip -- every probe retraces every frame for now
// (both deferred, see plan).
[shader("raygeneration")]
void DDGIProbeTraceRaygenShader()
{
	// Raygen shaders use CreateX()+a local, not the GetX() global convenience
	// wrapper -- GetX() is guarded out (NO_GLOBAL) for raygen compilation and
	// fails with "undeclared identifier" if used here (confirmed the hard
	// way), same pattern every raygen in raytracing.hlsl follows
	// (CreateVoxelOutput()/CreateVoxelScreen()).
	const DDGIProbeTraceData trace_data = CreateDDGIProbeTraceData();
	const DDGIInfo info = trace_data.GetInfo();
	const DDGIProbes probes = trace_data.GetProbes();
	const Raytracing raytracing = CreateRaytracing();

	uint2 atlas_texel = DispatchRaysIndex().xy;
	uint texel_size = info.GetAtlas_info().x;

	uint3 probe_coord = probes.ddgi_atlas_probe_coord(atlas_texel, texel_size, info.GetProbe_counts().x);

	// Residency early-out (see [[project-ddgi]] planning notes, DDGIProbeResidencyMark's
	// own comment): skip the TraceRay entirely for a probe nothing needs
	// this frame. No GPU-driven indirect DispatchRays exists in this
	// codebase's HAL, so this is a per-thread skip, not a shrunk dispatch --
	// still avoids the expensive part (the trace + hit shading). Leaves the
	// probe's existing radiance/gbuffer texels untouched rather than writing
	// zero, so a probe that stops being needed keeps its last valid value
	// for the cross-cascade fallback / for whenever it's reactivated.
	uint probe_linear_index = probes.ddgi_probe_linear_index(probe_coord, info.GetProbe_counts().xyz);
	uint probe_buffer_index = info.GetCascade_info().x + probe_linear_index;
	if (trace_data.GetProbe_residency()[probe_buffer_index] == 0)
		return;

	float2 local_uv = probes.ddgi_atlas_local_uv(atlas_texel, texel_size);
	float3 dir = ddgi_oct_decode(local_uv);

	float3 probe_pos = probes.ddgi_probe_world_pos(probe_coord, info.GetGrid_min().xyz, info.GetProbe_spacing().xyz, float3(0, 0, 0));

	[raypayload] RayPayload payload_gi;
	payload_gi.init();

	RayDesc ray;
	ray.Origin = probe_pos;
	ray.Direction = dir;
	ray.TMin = 0.05;
	ray.TMax = 10000.0;
	ColorPass(raytracing.GetScene(), ray, RAY_FLAG_NONE, payload_gi);

	float3 result_color = payload_gi.color.rgb;
	if (payload_gi.dist > 0.0 && info.GetFlags().x != 0)
	{
		float3 hit_pos = ray.Origin + ray.Direction * payload_gi.dist;
		float3 indirect = ddgi_sample_irradiance(hit_pos, payload_gi.hit_normal, info,
			trace_data.GetPrev_irradiance(), trace_data.GetPrev_visibility());
		result_color += payload_gi.albedo * indirect;
	}

	// atlas_texel is cascade-LOCAL (dispatch size is one cascade's own
	// DDGI_AtlasWidth x DDGI_AtlasHeight); add this cascade's own X-offset
	// to land the write in its slice of the shared, wider atlas.
	uint2 global_texel = atlas_texel + uint2(info.GetCascade_info().y, 0);
	trace_data.GetProbe_radiance()[global_texel] = float4(result_color, 1);
	trace_data.GetProbe_gbuffer()[global_texel] = float4(dir, payload_gi.dist);
}
