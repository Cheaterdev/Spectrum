export module HAL:Autogen.Tables.RayPayload;
import Core;
import :SIG;
import :Types;
import :HLSL;
import :Autogen.Tables.RayCone;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct RayPayload
	{
		static constexpr SlotID ID = SlotID::RayPayload;
		float4 color;
		float3 dir;
		uint recursion;
		float dist;
		RayCone cone;
		float4& GetColor() { return color; }
		float3& GetDir() { return dir; }
		uint& GetRecursion() { return recursion; }
		float& GetDist() { return dist; }
		RayCone& GetCone() { return cone; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(color);
			compiler.compile(dir);
			compiler.compile(recursion);
			compiler.compile(dist);
			compiler.compile(cone);
		}
		using Compiled = RayPayload;
		};
		#pragma pack(pop)
	}
