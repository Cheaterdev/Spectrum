#pragma once
#include "..\Tables\FrameInfo.h"
namespace Slots {
	struct FrameInfo:public DataHolder<Table::FrameInfo,FrameLayout::CameraData>
	{
		CB cb;
		SRV srv;
		FrameInfo(): DataHolder(cb,srv){}
		FrameInfo(const FrameInfo&other): DataHolder(cb,srv){cb = other.cb;srv = other.srv;}
	};
}
