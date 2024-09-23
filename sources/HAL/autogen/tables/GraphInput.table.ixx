export module HAL:Autogen.Tables.GraphInput;

import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
import <HAL.h>;
export namespace Table
{
	#pragma pack(push, 1)

	struct GraphInput
	{
		static constexpr SlotID ID = SlotID::GraphInput;
		uint3 dispatch_grid;
		uint3& GetDispatch_grid() { return dispatch_grid; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(dispatch_grid);
		}
		using Compiled = GraphInput;

	};
	#pragma pack(pop)
}

