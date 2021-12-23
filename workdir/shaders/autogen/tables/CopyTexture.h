#pragma once
struct CopyTexture_srv
{
	uint srcTex; // Texture2D<float4>
};
struct CopyTexture
{
	CopyTexture_srv srv;
	Texture2D<float4> GetSrcTex() { return ResourceDescriptorHeap[srv.srcTex]; }

};
 const CopyTexture CreateCopyTexture(CopyTexture_srv srv)
{
	const CopyTexture result = {srv
	};
	return result;
}
