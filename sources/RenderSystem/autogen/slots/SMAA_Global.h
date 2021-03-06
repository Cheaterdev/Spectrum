#pragma once
#include "..\Tables\SMAA_Global.h"
namespace Slots {
	struct SMAA_Global:public DataHolder<SMAA_Global, SlotID::SMAA_Global,Table::SMAA_Global,DefaultLayout::Instance0>
	{
		CB cb;
		SRV srv;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance0::CB_ID, DefaultLayout::Instance0::SRV_ID};
		SMAA_Global(): DataHolder(cb,srv){}
		SMAA_Global(const SMAA_Global&other): DataHolder(cb,srv){cb = other.cb;srv = other.srv;}
	};
}
