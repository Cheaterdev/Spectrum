#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Frustum
	{
		float4 planes[6];
		float4* GetPlanes() { return planes; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(planes);
		}
		using Compiled = Frustum;
		};
	#pragma pack(pop)
}
