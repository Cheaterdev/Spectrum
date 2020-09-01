#pragma once
#include "..\Tables\SMAA_Blend.h"
namespace Slots {
	struct SMAA_Blend:public DataHolder<Table::SMAA_Blend,DefaultLayout::Instance1>
	{
		SRV srv;
		SMAA_Blend(): DataHolder(srv){}
		SMAA_Blend(const SMAA_Blend&other): DataHolder(srv){srv = other.srv;}
	};
}
