#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct SMAA_Global
	{
		float4 subsampleIndices;
		float4 SMAA_RT_METRICS;
		Render::HLSL::Texture2D<float4> colorTex;
		Render::HLSL::Texture2D<float4>& GetColorTex() { return colorTex; }
		float4& GetSubsampleIndices() { return subsampleIndices; }
		float4& GetSMAA_RT_METRICS() { return SMAA_RT_METRICS; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(subsampleIndices);
			compiler.compile(SMAA_RT_METRICS);
			compiler.compile(colorTex);
		}
	};
	#pragma pack(pop)
}
