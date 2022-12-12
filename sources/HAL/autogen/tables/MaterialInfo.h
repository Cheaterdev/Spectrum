#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct MaterialInfo
	{
		std::vector<HLSL::Texture2D<float4>> textures;
		DynamicData data;
		std::vector<HLSL::Texture2D<float4>>& GetTextures() { return textures; }
		DynamicData& GetData() { return data; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(data);
			compiler.compile(textures);
		}
		};
	#pragma pack(pop)
}
