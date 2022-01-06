#pragma once
#include "..\Tables\Raytracing.h"
namespace Slots {
	struct Raytracing:public DataHolder<Raytracing, SlotID::Raytracing,Table::Raytracing,DefaultLayout::Raytracing>
	{
		Raytracing() = default;
	};
}
