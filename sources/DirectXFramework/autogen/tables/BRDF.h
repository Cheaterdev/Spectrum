#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct BRDF
	{
		Render::HLSL::RWTexture3D<float4> output;
		Render::HLSL::RWTexture3D<float4>& GetOutput() { return output; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(output);
		}
	};
	#pragma pack(pop)
}
