#pragma once
#include "..\Tables\EnvSource.h"
namespace Slots {
	struct EnvSource:public DataHolder<EnvSource, SlotID::EnvSource,Table::EnvSource,DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		EnvSource() = default;
	};
}
