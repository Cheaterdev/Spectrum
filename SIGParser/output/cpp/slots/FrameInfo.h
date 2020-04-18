#pragma once
#include "..\Tables\FrameInfo.h"
namespace Slots {
	struct FrameInfo:public DataHolder<Table::FrameInfo,FrameLayout::CameraData>
	{
		CB cb;
		FrameInfo(): DataHolder(cb){}
	};
}
