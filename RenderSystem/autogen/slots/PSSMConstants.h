#pragma once
#include "..\Tables\PSSMConstants.h"
namespace Slots {
	struct PSSMConstants:public DataHolder<Table::PSSMConstants,DefaultLayout::Instance0>
	{
		CB cb;
		PSSMConstants(): DataHolder(cb){}
		PSSMConstants(const PSSMConstants&other): DataHolder(cb){cb = other.cb;}
	};
}
