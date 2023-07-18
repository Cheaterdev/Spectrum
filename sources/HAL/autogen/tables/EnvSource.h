#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct EnvSource
	{
		static constexpr SlotID ID = SlotID::EnvSource;
		HLSL::TextureCube<float4> sourceTex;
		HLSL::TextureCube<float4>& GetSourceTex() { return sourceTex; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(sourceTex);
		}
		struct Compiled
		{
			uint sourceTex; // TextureCube<float4>
		};
	};
	#pragma pack(pop)
}
