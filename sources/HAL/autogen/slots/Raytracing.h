#pragma once
#include "..\Tables\Raytracing.h"
namespace Slots {
	struct Raytracing:public DataHolder<Raytracing, SlotID::Raytracing,Table::Raytracing,DefaultLayout::Raytracing>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		Raytracing() = default;
	};
}
