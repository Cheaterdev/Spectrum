export module HAL:Autogen.Tables.Instance;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct Instance
	{
		static constexpr SlotID ID = SlotID::Instance;
		uint instanceId;
		uint& GetInstanceId() { return instanceId; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(instanceId);
		}
		using Compiled = Instance;
		};
		#pragma pack(pop)
	}
