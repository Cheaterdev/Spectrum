#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct BRDF
	{
		static constexpr SlotID ID = SlotID::BRDF;
		HLSL::RWTexture3D<float4> output;
		HLSL::RWTexture3D<float4>& GetOutput() { return output; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(output);
		}
		struct Compiled
		{
			uint output; // RWTexture3D<float4>
		};
	};
	#pragma pack(pop)
}
