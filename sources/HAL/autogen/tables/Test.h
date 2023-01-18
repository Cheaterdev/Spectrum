#pragma once
#include "MeshInstanceInfo.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct Test
	{
		float data[16];
		std::vector<HLSL::StructuredBuffer<MeshInstanceInfo>> instances;
		std::vector<HLSL::Texture2D<float4>> tex;
		float* GetData() { return data; }
		std::vector<HLSL::StructuredBuffer<MeshInstanceInfo>>& GetInstances() { return instances; }
		std::vector<HLSL::Texture2D<float4>>& GetTex() { return tex; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(data);
			compiler.compile(instances);
			compiler.compile(tex);
		}
		struct Compiled
		{
			float data[16]; // float
			uint instances; // StructuredBuffer<MeshInstanceInfo>
			uint tex; // Texture2D<float4>
		};
	};
	#pragma pack(pop)
}
