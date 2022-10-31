#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct AABB
	{
		float4 min;
		float4 max;
		float4& GetMin() { return min; }
		float4& GetMax() { return max; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(min);
			compiler.compile(max);
		}
	};
	#pragma pack(pop)
}
