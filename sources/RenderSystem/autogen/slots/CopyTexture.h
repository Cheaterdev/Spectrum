#pragma once
#include "..\Tables\CopyTexture.h"
namespace Slots {
	struct CopyTexture:public DataHolder<SlotID::CopyTexture,Table::CopyTexture,DefaultLayout::Instance0>
	{
		SRV srv;
		CopyTexture(): DataHolder(srv){}
		CopyTexture(const CopyTexture&other): DataHolder(srv){srv = other.srv;}
	};
}
