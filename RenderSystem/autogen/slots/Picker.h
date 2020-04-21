#pragma once
#include "..\Tables\Picker.h"
namespace Slots {
	struct Picker:public DataHolder<Table::Picker,DefaultLayout::Instance0>
	{
		CB cb;
		UAV uav;
		Picker(): DataHolder(cb,uav){}
	};
}
