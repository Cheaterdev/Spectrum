#pragma once
#include "..\Tables\RaytracingRays.h"
namespace Slots {
	struct RaytracingRays:public DataHolder<RaytracingRays, SlotID::RaytracingRays,Table::RaytracingRays,DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		RaytracingRays() = default;
	};
}
