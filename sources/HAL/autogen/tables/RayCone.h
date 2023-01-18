#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct RayCone
	{
		float width;
		float angle;
		float& GetWidth() { return width; }
		float& GetAngle() { return angle; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(width);
			compiler.compile(angle);
		}
		using Compiled = RayCone;
		};
		#pragma pack(pop)
	}
