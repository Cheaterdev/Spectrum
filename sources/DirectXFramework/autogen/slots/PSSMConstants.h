#pragma once
#include "..\Tables\PSSMConstants.h"
namespace Slots {
	struct PSSMConstants:public DataHolder<PSSMConstants, SlotID::PSSMConstants,Table::PSSMConstants,DefaultLayout::Instance0>
	{
		CB cb;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance0::CB_ID};
		PSSMConstants(): DataHolder(cb){}
		PSSMConstants(const PSSMConstants&other): DataHolder(cb){cb = other.cb;}
	};
}
