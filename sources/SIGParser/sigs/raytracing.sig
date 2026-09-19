
# Mirrors RenderSystem::get().device().get_properties().rtx -- a fixed
# hardware capability, synced once (main.cpp's generate(), same spot
# UpscalerSelectors is synced) rather than recomputed by every pass that
# cares. Exists so [RenderCondition=...]/[SetupCondition=...] (this file's
# own RTXShadow, e.g.) can read it via get_context<Table::RenderDeviceCapabilities>()
# -- their generated setup() body lives in autogen/pass_defaults.cpp, a
# dedicated TU that doesn't import RenderSystem, only HAL (see that file's
# own comment for why a raw RenderSystem::get() call wouldn't be safe there).
struct RenderDeviceCapabilities
{
	bool rtx_supported = false;
	bool dlss_available = false;
	bool dlssrr_available = false;
}

[Bind = DefaultLayout::Instance2]
struct RaytracingRays
{
	GBuffer gbuffer;
	RWTexture2D<float4> output;
	float pixelAngle;
}

[Bind = DefaultLayout::Instance3]
struct ColorRTXOutput
{
	RWTexture2D<float4> output;
}

[nobind]
[raypayload]
struct RayCone
{
	[read = {anyhit,closesthit,miss,caller}]
	[write = {anyhit,closesthit,miss,caller}]
    float width;

	[read = {anyhit,closesthit,miss,caller}]
	[write = {anyhit,closesthit,miss,caller}]
    float angle;

	%{
	RayCone propagate(float surfaceSpreadAngle = 0, float hitT = 0)
	{
		RayCone result;
		result.width = width + angle* hitT;
		result.angle = angle + surfaceSpreadAngle;
		
		return result;
	}

	}%
}

[nobind]
[raypayload]
struct RayPayload
{
    RayCone cone;
	[read = {anyhit,closesthit,miss,caller}]
	[write = {anyhit,closesthit,miss,caller}]    
	float4 color;

	# Was `dir` -- confirmed unused anywhere (write-only, no read site) before
	# this repurposing, so renamed rather than adding a new field. Set by
	# MyClosestHitShader to the hit surface's shading normal; consumed by
	# DDGIProbeTrace to sample the probe-irradiance feedback term at the hit
	# point (see [[project-ddgi]] planning notes) -- not read anywhere else.
	[read = {anyhit,closesthit,miss,caller}]
	[write = {anyhit,closesthit,miss,caller}]
	float3 hit_normal;

	# Hit surface's diffuse albedo (post-metallic), same modulation
	# payload.color itself applies. Lets a caller add an indirect term
	# (irradiance * albedo) on top of payload.color's direct-only lighting
	# without payload.color already baking in NdotL/shadow terms that would
	# double-count. Only consumer today is DDGIProbeTrace's feedback sample.
	[read = {anyhit,closesthit,miss,caller}]
	[write = {anyhit,closesthit,miss,caller}]
	float3 albedo;

	[read = {anyhit,closesthit,miss,caller}]
	[write = {anyhit,closesthit,miss,caller}]
	uint recursion;

	[read = {anyhit,closesthit,miss,caller}]
	[write = {anyhit,closesthit,miss,caller}]
	float dist;

	%{

	RayPayload propagate(float surfaceSpreadAngle = 0, float hitT = 0)
	{
		RayPayload result;

		result.color = 0;
		result.dist = 0;
		result.recursion = recursion + 1;

		result.cone = cone.propagate(surfaceSpreadAngle, hitT);

		return result;
	}


	void init()
	{
		color = 0;
		albedo = 0;
		recursion = 0;
		dist = 0;
		cone.angle = 0;
		cone.width = 0;
	}

	}%
}

[nobind]
[raypayload]
struct ShadowPayload
{
	[read = {anyhit,closesthit,miss,caller}]
	[write = {anyhit,closesthit,miss,caller}]
	bool hit;

	# Distance to the occluder hit (RayTCurrent() at closesthit); < 0 signals
	# a miss. Used to pack a real SIGMA distanceToOccluder for NRD when the
	# RTX shadow reference is selected (see [[project-nrd-integration]]).
	[read = {closesthit,miss,caller}]
	[write = {closesthit,miss,caller}]
	float dist;
}

[nobind]
[raypayload]
struct ColorShadowPayload
{
	# Accumulated light transmittance toward the sun. Starts at 1 (fully lit);
	# an opaque hit zeroes it, a transparent hit tints/attenuates it.
	[read = {closesthit,miss,caller}]
	[write = {closesthit,miss,caller}]
	float3 transmittance;

	# Distance of the occluder hit; < 0 signals a miss (light reached). The caller
	# advances past each transparent hit and re-traces until opaque or miss.
	[read = {closesthit,miss,caller}]
	[write = {closesthit,miss,caller}]
	float dist;
}

[nobind]
struct Triangle
{
	mesh_vertex_input v;

	float lod;

	%{
		void init(mesh_vertex_input vertex0, mesh_vertex_input vertex1, mesh_vertex_input vertex2, float3 barycentrics)
		{
			v.normal = (vertex0.normal * barycentrics.x + vertex1.normal * barycentrics.y + vertex2.normal * barycentrics.z);
			v.tc = vertex0.tc * barycentrics.x + vertex1.tc * barycentrics.y + vertex2.tc * barycentrics.z;

			float P_a = length(cross(vertex2.pos - vertex0.pos, vertex1.pos - vertex0.pos));
			float T_a =  length(cross(float3(vertex2.tc - vertex0.tc,0), float3(vertex1.tc - vertex0.tc,0 )));
			lod = 0.5 * log2(T_a / P_a);

		}
	}%
}



[Bind = DefaultLayout::Raytracing]
struct Raytracing
{
	RaytracingAccelerationStructure scene;
}


RaytracePSO MainRTX
{
	root = DefaultLayout;


}

[Bind = MainRTX]
RaytraceRaygen Shadow
{
	[EntryPoint = ShadowRaygenShader]
	raygen = rtx/raytracing;
}

# Independent RTX-only reference: 1 ray per pixel, genuinely noisy soft
# shadow (no 16-sample averaging, no temporal history) -- see
# MyRaygenShaderShadowRTXOnly's doc comment in raytracing.hlsl and PassNode
# ShadowRTX in voxel.sig.
[Bind = MainRTX]
RaytraceRaygen ShadowRTX
{
	[EntryPoint = MyRaygenShaderShadowRTXOnly]
	raygen = rtx/raytracing;
}


# RTX-only reflection raygen -- see MyRaygenShaderReflectionRTXOnly's doc
# comment in raytracing.hlsl and PassNode ReflectionRTX in voxel.sig.
[Bind = MainRTX]
RaytraceRaygen ReflectionRTX
{
	[EntryPoint = MyRaygenShaderReflectionRTXOnly]
	raygen = rtx/raytracing;
}

# Half-res sibling for PassNode ReflectionRTXHalf (voxel.sig) -- same trace
# logic, shared via TraceReflection() in raytracing.hlsl, just over the
# half-res GBuffer instead of full res.
[Bind = MainRTX]
RaytraceRaygen ReflectionRTXHalf
{
	[EntryPoint = MyRaygenShaderReflectionRTXHalfRes]
	raygen = rtx/raytracing;
}

# Voxel-cone-traced reflection signal, selectable against ReflectionRTX as
# NRD's input -- see MyRaygenShaderReflection's doc comment in
# raytracing.hlsl and PassNode ScreenReflection in voxel.sig.
[Bind = MainRTX]
RaytraceRaygen Reflection
{
	[EntryPoint = MyRaygenShaderReflection]
	raygen = rtx/raytracing;
}

# RTX-only reference: 1 ray per pixel, genuinely noisy diffuse
# GI (no voxel-cone-trace fallback on miss, no temporal history) -- see
# MyRaygenShaderIndirectRTXOnly's doc comment in raytracing.hlsl and
# PassNode IndirectRTX in voxel.sig.
[Bind = MainRTX]
RaytraceRaygen IndirectRTX
{
	[EntryPoint = MyRaygenShaderIndirectRTXOnly]
	raygen = rtx/raytracing;
}

# Half-res sibling for PassNode IndirectRTXHalf (voxel.sig) -- same trace
# logic, shared via TraceIndirectDiffuse() in raytracing.hlsl, just over the
# half-res GBuffer instead of full res.
[Bind = MainRTX]
RaytraceRaygen IndirectRTXHalf
{
	[EntryPoint = MyRaygenShaderIndirectRTXHalfRes]
	raygen = rtx/raytracing;
}

# Voxel-cone-traced indirect-GI signal, selectable against IndirectRTX as
# NRD's input -- see MyRaygenShader's doc comment in raytracing.hlsl and
# PassNode VoxelScreen in voxel.sig.
[Bind = MainRTX]
RaytraceRaygen Indirect
{
	[EntryPoint = MyRaygenShader]
	raygen = rtx/raytracing;
}

[Bind = MainRTX]
RaytraceRaygen ColorRTX
{
	[EntryPoint = ColorRTXRaygenShader]
	raygen = rtx/raytracing_debug;
}

# DDGI probe-volume trace raygen (ddgi.sig, see [[project-ddgi]] planning
# notes) -- declared here rather than in ddgi.sig itself: RaytraceRaygen::ID
# is assigned per-.sig-file by the generator, not globally, so a raygen
# declared in a separate file collides with an existing ID (confirmed the
# hard way: DDGIProbeTrace and Shadow both got ID=0, which fails RTX.ixx's
# dispatch<T> static_assert for every RTX pass, not just this one). Keep any
# future raygen in this same file for the same reason.
[Bind = MainRTX]
RaytraceRaygen DDGIProbeTrace
{
	[EntryPoint = DDGIProbeTraceRaygenShader]
	raygen = ddgi/ddgi_probe_trace;
}

[Bind = MainRTX]
RaytracePass ShadowPass
{
	[EntryPoint = ShadowMissShader]
	miss = rtx/raytracing;

	[EntryPoint = ShadowClosestHitShader]
	closest_hit = rtx/raytracing;

	payload = ShadowPayload;
}

[Bind = MainRTX]
RaytracePass ColorPass
{
	[EntryPoint = MyMissShader]
	miss = rtx/raytracing;

	[EntryPoint = MyClosestHitShader]
	closest_hit = none;

	# Real DXR any-hit shader (not the inline-RayQuery candidate-loop
	# workaround VSM's own verify ray uses -- TraceRay()'s hit groups have a
	# local root signature reaching this material's own MaterialInfo/
	# compiled opacity graph directly, which inline ray tracing can't).
	# Per-material like closest_hit above, same reason: needs this specific
	# material's COMPILED_FUNC to evaluate its actual opacity.
	[EntryPoint = MyAnyHitShader]
	any_hit = none;

	payload = RayPayload;

	local = MaterialInfo;
	per_material = true;
}

# Transparent-aware shadow ray: per-material so each material's closest-hit knows
# its own transparency (opaque -> block, transparent -> tint + continue). Used by
# the RTX color hit shader only; the binary ShadowPass above is left untouched.
[Bind = MainRTX]
RaytracePass ColorShadowPass
{
	[EntryPoint = ColorShadowMissShader]
	miss = rtx/raytracing;

	[EntryPoint = ColorShadowClosestHitShader]
	closest_hit = none;

	# See ColorPass's own comment on any_hit above -- same reasoning, this
	# pass's shadow rays need real per-material cutout support too.
	[EntryPoint = ColorShadowAnyHitShader]
	any_hit = none;

	payload = ColorShadowPayload;

	local = MaterialInfo;
	per_material = true;
}



[Static]
[Compute]
# ShadowMask is create()'d unconditionally (via [Always]/[Size]/[Format]
# below), regardless of RTX support, matching the pre-existing behavior
# (render() only actually writes it under RTX; other passes already guard on
# builder.exists() before reading it). [RenderCondition] alone (no
# [SetupCondition]): IgnoreRender, not Disabled, when RTX isn't supported --
# setup() still needs to run every frame so ShadowMask keeps existing.
[RenderCondition = RenderDeviceCapabilities::rtx_supported]
PassNode RTXShadow
{
	# Flat fields, not the (removed) GBuffer PassView -- see pssm.sig's own
	# comment.
	[Always = Read] Texture GBuffer_Albedo;
	[Always = Read] Texture GBuffer_Normals;
	[Always = Read] Texture GBuffer_Depth;
	[Always = Read] Texture GBuffer_Specular;
	[Always = Read] Texture GBuffer_Speed;
	[Always = None] Texture GBuffer_DepthMips;
	[Always = Read] Texture GBuffer_DepthPrev;
	[Always = UnorderedAccess] [Size = ViewportContext::frame_size] [Format = R16G16B16A16_FLOAT] Texture ShadowMask;
	[Always = UnorderedAccess] [Size = `(size_t)Constants::WG_TileSection`] ByteAdressBuffer WorkGraphBuffer;
}

# Debug reference mode for RTXShadow (see RTX::debug_full_reference_shadow
# in RTX.ixx): a genuine 16-ray soft-shadow computation, entirely separate
# from the Bend/FFX hybrid-shadow-denoiser dispatch RTXShadow normally runs
# -- ground truth to compare VSM's own PCSS approximation against. Bound at
# the same Instance2 slot the Bend path's own DispatchParameters (SS_Shadow.
# sig) uses, since RTXShadow::render() only ever binds one or the other per
# frame, never both at once.
[Bind = DefaultLayout::Instance2]
struct RTXShadowReference
{
	GBuffer gbuffer;
	RWTexture2D<float4> output;
}

ComputePSO RTXShadowReferenceCompute
{
	root = DefaultLayout;

	[EntryPoint = CS_REFERENCE]
	compute = rtx/rtx_shadow_reference;
}

[Static]
[Compute]
[SetupCondition = RenderDeviceCapabilities::rtx_supported]
PassNode RTXColorPass
{
	# Read-only dependency on PreScene so the RTX BVH is built/updated before tracing.
	[Always = Read] StructuredBuffer<uint> scene;
	# Force the sky chain to run so FrameInfo.GetSky() is populated for the miss shader.
	[Always = Read] TextureCube sky_cubemap_filtered;
	[Always = UnorderedAccess] [Size = ViewportContext::frame_size] [Format = R16G16B16A16_FLOAT] Texture ColorOutput;
}