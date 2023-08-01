export module HAL:Autogen.Tables.SMAA_Global;
import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct SMAA_Global
	{
		static constexpr SlotID ID = SlotID::SMAA_Global;
		float4 subsampleIndices;
		float4 SMAA_RT_METRICS;
		HLSL::Texture2D<float4> colorTex;
		HLSL::Texture2D<float4>& GetColorTex() { return colorTex; }
		float4& GetSubsampleIndices() { return subsampleIndices; }
		float4& GetSMAA_RT_METRICS() { return SMAA_RT_METRICS; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(subsampleIndices);
			compiler.compile(SMAA_RT_METRICS);
			compiler.compile(colorTex);
		}
		struct Compiled
		{
			float4 subsampleIndices; // float4
			float4 SMAA_RT_METRICS; // float4
			uint colorTex; // Texture2D<float4>
		};
	};
	#pragma pack(pop)
}
