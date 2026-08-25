
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

	[read = {anyhit,closesthit,miss,caller}]
	[write = {anyhit,closesthit,miss,caller}]	
	float3 dir;

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
	raygen = raytracing;
}


[Bind = MainRTX]
RaytraceRaygen Reflection
{
	[EntryPoint = MyRaygenShaderReflection]
	raygen = raytracing;
}


[Bind = MainRTX]
RaytraceRaygen Indirect
{
	[EntryPoint = MyRaygenShader]
	raygen = raytracing;
}

[Bind = MainRTX]
RaytraceRaygen ColorRTX
{
	[EntryPoint = ColorRTXRaygenShader]
	raygen = raytracing_debug;
}

[Bind = MainRTX]
RaytracePass ShadowPass
{
	[EntryPoint = ShadowMissShader]
	miss = raytracing;

	[EntryPoint = ShadowClosestHitShader]
	closest_hit = raytracing;

	payload = ShadowPayload;
}

[Bind = MainRTX]
RaytracePass ColorPass
{
	[EntryPoint = MyMissShader]
	miss = raytracing;

	[EntryPoint = MyClosestHitShader]
	closest_hit = none;

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
	miss = raytracing;

	[EntryPoint = ColorShadowClosestHitShader]
	closest_hit = none;

	payload = ColorShadowPayload;

	local = MaterialInfo;
	per_material = true;
}



[Static]
[Compute]
PassNode RTXShadow
{
	GBuffer gbuffer;
	[Write] Texture ShadowMask;
	[Write] ByteAdressBuffer WorkGraphBuffer;
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
	compute = RTXShadowReference;
}

[Static]
[Compute]
PassNode RTXColorPass
{
	# Read-only dependency on PreScene so the RTX BVH is built/updated before tracing.
	StructuredBuffer<uint> scene;
	# Force the sky chain to run so FrameInfo.GetSky() is populated for the miss shader.
	TextureCube sky_cubemap_filtered;
	[Write] Texture ColorOutput;
}