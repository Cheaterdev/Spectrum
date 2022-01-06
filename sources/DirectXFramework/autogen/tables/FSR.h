#pragma once
#include "FSRConstants.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct FSR
	{
		Render::HLSL::Texture2D<float4> source;
		Render::HLSL::RWTexture2D<float4> target;
		FSRConstants constants;
		Render::HLSL::Texture2D<float4>& GetSource() { return source; }
		Render::HLSL::RWTexture2D<float4>& GetTarget() { return target; }
		FSRConstants& GetConstants() { return constants; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(source);
			compiler.compile(target);
			compiler.compile(constants);
		}
	};
	#pragma pack(pop)
}
