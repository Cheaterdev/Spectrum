#pragma once
#include "..\Tables\FrameInfo.h"
namespace Slots {
	struct FrameInfo:public DataHolder<FrameInfo, SlotID::FrameInfo,Table::FrameInfo,FrameLayout::CameraData>
	{
		FrameInfo() = default;
	};
}
