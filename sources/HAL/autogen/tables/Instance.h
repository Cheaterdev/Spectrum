#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Instance
	{
		static constexpr SlotID ID = SlotID::Instance;
		uint instanceId;
		uint& GetInstanceId() { return instanceId; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(instanceId);
		}
		using Compiled = Instance;
		};
		#pragma pack(pop)
	}
