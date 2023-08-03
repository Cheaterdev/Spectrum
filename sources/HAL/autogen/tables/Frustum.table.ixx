export module HAL:Autogen.Tables.Frustum;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct Frustum
	{
		static constexpr SlotID ID = SlotID::Frustum;
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
