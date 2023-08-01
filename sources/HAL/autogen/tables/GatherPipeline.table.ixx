export module HAL:Autogen.Tables.GatherPipeline;
import Core;
import :SIG;
import :Types;
import :HLSL;
import :Autogen.Tables.CommandData;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct GatherPipeline
	{
		static constexpr SlotID ID = SlotID::GatherPipeline;
		uint4 pip_ids[2];
		HLSL::AppendStructuredBuffer<CommandData> commands[8];
		uint4* GetPip_ids() { return pip_ids; }
		HLSL::AppendStructuredBuffer<CommandData>* GetCommands() { return commands; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(pip_ids);
			compiler.compile(commands);
		}
		struct Compiled
		{
			uint4 pip_ids[2]; // uint4
			uint commands[8]; // AppendStructuredBuffer<CommandData>
		};
	};
	#pragma pack(pop)
}
