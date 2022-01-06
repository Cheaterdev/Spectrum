#pragma once
#include "..\Tables\FrameClassification.h"
namespace Slots {
	struct FrameClassification:public DataHolder<FrameClassification, SlotID::FrameClassification,Table::FrameClassification,DefaultLayout::Instance2>
	{
		FrameClassification() = default;
	};
}
