#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct MaterialInfo
	{
		uint textureOffset;
		DynamicData data;
		uint& GetTextureOffset() { return textureOffset; }
		DynamicData& GetData() { return data; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(data);
			compiler.compile(textureOffset);
		}
	};
	#pragma pack(pop)
}
