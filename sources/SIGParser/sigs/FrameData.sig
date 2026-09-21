# Debug-only bitmask, shared FrameInfo-wide rather than living on any one
# system's own struct (RTX ray-miss behavior isn't DDGI-, IndirectRTX-, or
# ReflectionRTX-exclusive -- MyMissShader (raytracing.hlsl) services all of
# them through one shared miss shader with no way to know which). Real SIG
# enum instead of a raw uint + hand-typed magic numbers, same reasoning
# DDGIControlFlags (ddgi.sig) already established -- explicit power-of-two
# values since these combine as a bitmask, not picked one-at-a-time.
enum RTXDebugFlags
{
	DisableSkyFallback = 1;
}

struct ViewportContext
{
	int2 frame_size;
	int2 upscale_size;
}

struct Frustum
{
	float4 planes[6];
}

struct Camera
{
	float4x4 view;
	float4x4 proj;
	float4x4 viewProj;
	float4x4 invView;
	float4x4 invProj;
	float4x4 invViewProj;
	float4x4 reprojectionProj;
	float4 position;
	float4 direction;
	float4 jitter;
	
	Frustum frustum;
}

[Bind = FrameLayout::CameraData]
struct FrameInfo
{
	float4 time;

    Camera camera;
    Camera prevCamera;

  	Texture2D<float4> bestFitNormals;
	Texture3D<float4> brdf;
	TextureCube<float4> sky;
	float4 sunDir;

	# Texture LOD bias compensating for rendering at frame_size and upscaling
	# to upscale_size (FSR/DLSS). 0 when no upscaler is active, and the
	# correct default for every OTHER pipeline that fills its own FrameInfo
	# (AssetRenderer, PSSM, StencilRenderer) without touching this field --
	# unset, it was garbage, and SampleBias() read that garbage as an LOD
	# offset, sampling the smallest/flattest mip (trashed material colors).
	float mipBias = 0;

	# Hi-Z pyramid for per-meshlet occlusion (built in MeshRenderer.cpp).
	# Whether it is USED is a PSO permutation, not a runtime flag -- see
	# GBufferDraw's HiZOcclusion define in scene.sig.
	#
	# [Auto]: passes that render without a Hi-Z (the asset thumbnail renderer,
	# AssetGBuffer) leave this unset, and the mesh shader samples it
	# unconditionally. Unset it read descriptor 0 -- which holds a BUFFER --
	# as a Texture2D (GBV #940): undefined, and capable of OVER-culling, i.e.
	# silently dropping geometry from thumbnails.
	#
	# Zero is the correct fallback rather than merely a safe one: the test is
	# `near_clip.z < min(taps)` under reversed-Z, so all-zero taps make it
	# false and nothing is culled, which is what "no Hi-Z available" means.
	[Auto = Texture_Null]
	Texture2D<float> mainHiZ;

	# RTXDebugFlags bitmask (above). Mirrored from DDGI.ixx's own
	# ddgi_sky_fallback_disabled(), the one place DisableSkyFallback is
	# actually toggled from today, even though its effect isn't
	# DDGI-exclusive -- see that flag's own comment.
	uint debugFlags = 0;

	# VSM's lean world-position shadow lookup (vsm.sig's own struct, used
	# outside VSM's own passes -- see its doc comment there), bound globally
	# here rather than per-pass like VoxelGI::Lighting's own VSMShadowLookup
	# field: the one reader that actually needs it, MyClosestHitShader
	# (universal_material_raytracing.hlsl), is a SHARED hit-group shader
	# reached from many differently-shaped binding structs (DDGIProbeTraceData,
	# VoxelOutput, GBuffer...), so there's no single caller-owned struct to
	# embed it in the way Lighting could. Filled once per frame in main.cpp's
	# own FrameInfo-population lambda (same spot mainHiZ/debugFlags are),
	# from VSM::fill_shadow_lookup_constants -- see RayPayload::use_vsm_shadow
	# (raytracing.sig) for who actually opts into reading it (DDGIProbeTrace,
	# ddgi_probe_trace.hlsl, as of this comment -- see [[project-ddgi]]
	# planning notes for why: replaces a second recursive RTX shadow ray per
	# probe texel with one cheap 3x3 hardware-PCF tap).
	VSMShadowLookup vsm;


	%{
		float2 IntegrateBRDF(float Roughness, float Metallic, float NoV)
		{
			return GetBrdf().SampleLevel(linearClampSampler, float3(Roughness, Metallic, 0.5 + 0.5 * NoV), 0);
		}

		
		half3 compress_normals(inout half3 vNormal) 
		{
			// renormalize (needed only if any blending or interpolation happened before)
			vNormal.rgb = normalize(vNormal.rgb);
			// get unsigned normal for the cubemap lookup
			half3  vNormalUns = abs(vNormal.rgb);
			// get the main axis for cubemap lookup
			half   maxNAbs = max(vNormalUns.z, max(vNormalUns.x, vNormalUns.y));
			// get texture coordinates in a collapsed cubemap
			float2 vTexCoord = vNormalUns.z<maxNAbs ? (vNormalUns.y<maxNAbs ? vNormalUns.yz : vNormalUns.xz) : vNormalUns.xy;
			vTexCoord = vTexCoord.x < vTexCoord.y ? vTexCoord.yx : vTexCoord.xy;
			vTexCoord.y /= vTexCoord.x;
			// fit normal into the edge of unit cube
			vNormal.rgb /= maxNAbs;
			// look-up fitting length and scale the normal to get the best fit
			half fFittingScale = GetBestFitNormals().SampleLevel(pointClampSampler, vTexCoord,0).a;
			// scale the normal to get the best fit
			vNormal.rgb *= fFittingScale; 
			// wrap to [0;1] unsigned form
			vNormal.rgb = vNormal.rgb * .5h + .5h;

			return vNormal;
		}	


	}%


}
