#pragma once
#include "..\Tables\EnvSource.h"
namespace Slots {
	struct EnvSource:public DataHolder<EnvSource, SlotID::EnvSource,Table::EnvSource,DefaultLayout::Instance2>
	{
		SRV srv;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance2::SRV_ID};
		EnvSource(): DataHolder(srv){}
		EnvSource(const EnvSource&other): DataHolder(srv){srv = other.srv;}
	};
}
