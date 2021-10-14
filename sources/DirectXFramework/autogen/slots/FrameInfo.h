#pragma once
#include "..\Tables\FrameInfo.h"
namespace Slots {
	struct FrameInfo:public DataHolder<FrameInfo, SlotID::FrameInfo,Table::FrameInfo,FrameLayout::CameraData>
	{
		CB cb;
		SRV srv;
		static inline const std::vector<UINT> tables = {FrameLayout::CameraData::CB_ID, FrameLayout::CameraData::SRV_ID};
		FrameInfo(): DataHolder(cb,srv){}
		FrameInfo(const FrameInfo&other): DataHolder(cb,srv){cb = other.cb;srv = other.srv;}
	};
}
