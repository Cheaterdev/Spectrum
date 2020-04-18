#pragma once
#include "..\Tables\MipMapping.h"
namespace Slots {
	struct MipMapping:public DataHolder<Table::MipMapping,DefaultLayout::Instance0>
	{
		CB cb;
		SRV srv;
		UAV uav;
		MipMapping(): DataHolder(cb,srv,uav){}
	};
}
