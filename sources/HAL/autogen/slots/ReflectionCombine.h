#pragma once
#include "..\Tables\ReflectionCombine.h"
namespace Slots {
	struct ReflectionCombine:public DataHolder<ReflectionCombine, SlotID::ReflectionCombine,Table::ReflectionCombine,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		ReflectionCombine() = default;
	};
}
