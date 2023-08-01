export module HAL:Autogen.Tables.Raytracing;
import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct Raytracing
	{
		static constexpr SlotID ID = SlotID::Raytracing;
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