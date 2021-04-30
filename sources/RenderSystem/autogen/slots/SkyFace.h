#pragma once
#include "..\Tables\SkyFace.h"
namespace Slots {
	struct SkyFace:public DataHolder<SkyFace, SlotID::SkyFace,Table::SkyFace,DefaultLayout::Instance1>
	{
		CB cb;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance1::CB_ID};
		SkyFace(): DataHolder(cb){}
		SkyFace(const SkyFace&other): DataHolder(cb){cb = other.cb;}
	};
}
