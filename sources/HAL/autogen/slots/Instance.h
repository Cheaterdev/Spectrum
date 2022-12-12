#pragma once
#include "..\Tables\Instance.h"
namespace Slots {
	struct Instance:public DataHolder<Instance, SlotID::Instance,Table::Instance,DefaultLayout::Instance3>
	{
		Instance() = default;
	};
}
