#pragma once
#include "..\Tables\DenoiserReflectionCommon.h"
namespace Slots {
	struct DenoiserReflectionCommon:public DataHolder<DenoiserReflectionCommon, SlotID::DenoiserReflectionCommon,Table::DenoiserReflectionCommon,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DenoiserReflectionCommon() = default;
	};
}
