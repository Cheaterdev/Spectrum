#pragma once
#include "Camera.h"
struct FrameInfo_cb
{
	float4 time; // float4
	float4 sunDir; // float4
	Camera_cb camera;
	Camera_cb prevCamera;
};
struct FrameInfo_srv
{
	uint bestFitNormals; // Texture2D<float4>
	uint brdf; // Texture3D<float4>
	uint sky; // TextureCube<float4>
};
struct FrameInfo
{
	FrameInfo_cb cb;
	FrameInfo_srv srv;
	Texture2D<float4> GetBestFitNormals() { return ResourceDescriptorHeap[srv.bestFitNormals]; }
	Texture3D<float4> GetBrdf() { return ResourceDescriptorHeap[srv.brdf]; }
	TextureCube<float4> GetSky() { return ResourceDescriptorHeap[srv.sky]; }
	float4 GetTime() { return cb.time; }
	float4 GetSunDir() { return cb.sunDir; }
	Camera GetCamera() { return CreateCamera(cb.camera); }
	Camera GetPrevCamera() { return CreateCamera(cb.prevCamera); }

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


	
};
 const FrameInfo CreateFrameInfo(FrameInfo_cb cb,FrameInfo_srv srv)
{
	const FrameInfo result = {cb,srv
	};
	return result;
}
