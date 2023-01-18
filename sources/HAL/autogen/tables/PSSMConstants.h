#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct PSSMConstants
	{
		int level;
		float time;
		int& GetLevel() { return level; }
		float& GetTime() { return time; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(level);
			compiler.compile(time);
		}
		using Compiled = PSSMConstants;
		};
		#pragma pack(pop)
	}
