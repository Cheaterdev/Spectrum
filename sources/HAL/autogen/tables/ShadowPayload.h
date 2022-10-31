#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct ShadowPayload
	{
		bool hit;
		bool& GetHit() { return hit; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(hit);
		}
	};
	#pragma pack(pop)
}
