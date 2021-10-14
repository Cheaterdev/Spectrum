#pragma once
#include "..\Tables\SMAA_Blend.h"
namespace Slots {
	struct SMAA_Blend:public DataHolder<SMAA_Blend, SlotID::SMAA_Blend,Table::SMAA_Blend,DefaultLayout::Instance1>
	{
		SRV srv;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance1::SRV_ID};
		SMAA_Blend(): DataHolder(srv){}
		SMAA_Blend(const SMAA_Blend&other): DataHolder(srv){srv = other.srv;}
	};
}
