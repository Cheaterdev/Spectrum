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
