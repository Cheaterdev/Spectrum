#pragma once
struct CopyTexture_srv
{
	Texture2D<float4> srcTex;
};
struct CopyTexture
{
	CopyTexture_srv srv;
	Texture2D<float4> GetSrcTex() { return srv.srcTex; }

};
 const CopyTexture CreateCopyTexture(CopyTexture_srv srv)
{
	const CopyTexture result = {srv
	};
	return result;
}
