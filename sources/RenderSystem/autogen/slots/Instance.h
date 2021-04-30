#pragma once
#include "..\Tables\Instance.h"
namespace Slots {
	struct Instance:public DataHolder<Instance, SlotID::Instance,Table::Instance,DefaultLayout::Instance2>
	{
		CB cb;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance2::CB_ID};
		Instance(): DataHolder(cb){}
		Instance(const Instance&other): DataHolder(cb){cb = other.cb;}
	};
}
