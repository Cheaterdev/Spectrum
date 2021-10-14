#pragma once
#include "..\Tables\Countour.h"
namespace Slots {
	struct Countour:public DataHolder<Countour, SlotID::Countour,Table::Countour,DefaultLayout::Instance0>
	{
		CB cb;
		SRV srv;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance0::CB_ID, DefaultLayout::Instance0::SRV_ID};
		Countour(): DataHolder(cb,srv){}
		Countour(const Countour&other): DataHolder(cb,srv){cb = other.cb;srv = other.srv;}
	};
}
