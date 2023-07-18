#pragma once
#include "..\Tables\Instance.h"
namespace Slots {
	struct Instance:public DataHolder<Instance, SlotID::Instance,Table::Instance,DefaultLayout::Instance3>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		Instance() = default;
	};
}
