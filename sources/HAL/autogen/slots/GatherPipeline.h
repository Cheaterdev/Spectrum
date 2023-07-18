#pragma once
#include "..\Tables\GatherPipeline.h"
namespace Slots {
	struct GatherPipeline:public DataHolder<GatherPipeline, SlotID::GatherPipeline,Table::GatherPipeline,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		GatherPipeline() = default;
	};
}
