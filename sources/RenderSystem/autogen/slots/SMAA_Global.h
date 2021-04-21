#pragma once
#include "..\Tables\SMAA_Global.h"
namespace Slots {
	struct SMAA_Global:public DataHolder<SlotID::SMAA_Global,Table::SMAA_Global,DefaultLayout::Instance0>
	{
		CB cb;
		SRV srv;
		SMAA_Global(): DataHolder(cb,srv){}
		SMAA_Global(const SMAA_Global&other): DataHolder(cb,srv){cb = other.cb;srv = other.srv;}
	};
}
