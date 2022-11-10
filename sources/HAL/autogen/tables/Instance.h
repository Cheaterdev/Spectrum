#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Instance
	{
		uint instanceId;
		uint& GetInstanceId() { return instanceId; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(instanceId);
		}
	};
	#pragma pack(pop)
}
