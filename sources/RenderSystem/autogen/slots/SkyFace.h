#pragma once
#include "..\Tables\SkyFace.h"
namespace Slots {
	struct SkyFace:public DataHolder<SlotID::SkyFace,Table::SkyFace,DefaultLayout::Instance1>
	{
		CB cb;
		SkyFace(): DataHolder(cb){}
		SkyFace(const SkyFace&other): DataHolder(cb){cb = other.cb;}
	};
}
