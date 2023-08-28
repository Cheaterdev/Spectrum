export module HAL:Autogen.Tables.FSRConstants;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct FSRConstants
	{
		static constexpr SlotID ID = SlotID::FSRConstants;
		uint4 Const0;
		uint4 Const1;
		uint4 Const2;
		uint4 Const3;
		uint4 Sample;
		uint4& GetConst0() { return Const0; }
		uint4& GetConst1() { return Const1; }
		uint4& GetConst2() { return Const2; }
		uint4& GetConst3() { return Const3; }
		uint4& GetSample() { return Sample; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(Const0);
			compiler.compile(Const1);
			compiler.compile(Const2);
			compiler.compile(Const3);
			compiler.compile(Sample);
		}
		using Compiled = FSRConstants;
		};
		#pragma pack(pop)
	}
