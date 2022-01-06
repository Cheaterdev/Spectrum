#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct FontRendering
	{
		Render::HLSL::Texture2D<float> tex0;
		Render::HLSL::Buffer<float4> positions;
		Render::HLSL::Texture2D<float>& GetTex0() { return tex0; }
		Render::HLSL::Buffer<float4>& GetPositions() { return positions; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(tex0);
			compiler.compile(positions);
		}
	};
	#pragma pack(pop)
}
