#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Raytracing
	{
		HLSL::RaytracingAccelerationStructure scene;
		HLSL::RaytracingAccelerationStructure& GetScene() { return scene; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(scene);
		}
		struct Compiled
		{
			uint scene; // RaytracingAccelerationStructure
		};
	};
	#pragma pack(pop)
}
