#pragma once
#include "..\Tables\CopyTexture.h"
namespace Slots {
	struct CopyTexture:public DataHolder<Table::CopyTexture,DefaultLayout::Instance0>
	{
		SRV srv;
		CopyTexture(): DataHolder(srv){}
	};
}
