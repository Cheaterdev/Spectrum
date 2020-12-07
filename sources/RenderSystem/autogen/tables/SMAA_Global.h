#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct SMAA_Global
	{
		struct CB
		{
			float4 subsampleIndices;
			float4 SMAA_RT_METRICS;
		} &cb;
		struct SRV
		{
			Render::HLSL::Texture2D<float4> colorTex;
		} &srv;
		Render::HLSL::Texture2D<float4>& GetColorTex() { return srv.colorTex; }
		float4& GetSubsampleIndices() { return cb.subsampleIndices; }
		float4& GetSMAA_RT_METRICS() { return cb.SMAA_RT_METRICS; }
		SMAA_Global(CB&cb,SRV&srv) :cb(cb),srv(srv){}
	};
	#pragma pack(pop)
}
