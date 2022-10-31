#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Test
	{
		float data[16];
		std::vector<HLSL::Texture2D<float4>> tex;
		float* GetData() { return data; }
		std::vector<HLSL::Texture2D<float4>>& GetTex() { return tex; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(data);
			compiler.compile(tex);
		}
	};
	#pragma pack(pop)
}
