#pragma once
struct SMAA_Resolve_srv
{
	Texture2D blendTex;
};
struct SMAA_Resolve
{
	SMAA_Resolve_srv srv;
	Texture2D GetBlendTex() { return srv.blendTex; }

};
 const SMAA_Resolve CreateSMAA_Resolve(SMAA_Resolve_srv srv)
{
	const SMAA_Resolve result = {srv
	};
	return result;
}
