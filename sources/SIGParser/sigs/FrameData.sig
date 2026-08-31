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
