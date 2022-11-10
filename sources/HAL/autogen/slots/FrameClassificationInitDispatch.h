#pragma once
#include "..\Tables\FrameClassificationInitDispatch.h"
namespace Slots {
	struct FrameClassificationInitDispatch:public DataHolder<FrameClassificationInitDispatch, SlotID::FrameClassificationInitDispatch,Table::FrameClassificationInitDispatch,DefaultLayout::Instance2>
	{
		FrameClassificationInitDispatch() = default;
	};
}
