#pragma once
#include "..\Tables\SkyFace.h"
namespace Slots {
	struct SkyFace:public DataHolder<Table::SkyFace,DefaultLayout::Instance1>
	{
		CB cb;
		SkyFace(): DataHolder(cb){}
	};
}
