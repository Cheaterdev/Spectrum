#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct DebugStruct
	{
		uint4 v;
		uint4& GetV() { return v; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(v);
		}
		using Compiled = DebugStruct;
		};
	#pragma pack(pop)
}
