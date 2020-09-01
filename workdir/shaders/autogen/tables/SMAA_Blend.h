#pragma once
struct SMAA_Blend_srv
{
	Texture2D blendTex;
};
struct SMAA_Blend
{
	SMAA_Blend_srv srv;
	Texture2D GetBlendTex() { return srv.blendTex; }

};
 const SMAA_Blend CreateSMAA_Blend(SMAA_Blend_srv srv)
{
	const SMAA_Blend result = {srv
	};
	return result;
}
