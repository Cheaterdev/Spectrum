#pragma once
#include "..\Tables\CopyTexture.h"
namespace Slots {
	struct CopyTexture:public DataHolder<CopyTexture, SlotID::CopyTexture,Table::CopyTexture,DefaultLayout::Instance0>
	{
		SRV srv;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance0::SRV_ID};
		CopyTexture(): DataHolder(srv){}
		CopyTexture(const CopyTexture&other): DataHolder(srv){srv = other.srv;}
	};
}
