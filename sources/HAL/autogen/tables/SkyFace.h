#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct SkyFace
	{
		uint face;
		uint& GetFace() { return face; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(face);
		}
		using Compiled = SkyFace;
		};
		#pragma pack(pop)
	}
