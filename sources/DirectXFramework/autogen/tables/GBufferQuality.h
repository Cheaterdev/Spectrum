#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct GBufferQuality
	{
		HLSL::Texture2D<float4> ref;
		HLSL::Texture2D<float4>& GetRef() { return ref; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(ref);
		}
	};
	#pragma pack(pop)
}
