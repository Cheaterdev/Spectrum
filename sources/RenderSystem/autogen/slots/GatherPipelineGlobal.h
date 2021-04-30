#pragma once
#include "..\Tables\GatherPipelineGlobal.h"
namespace Slots {
	struct GatherPipelineGlobal:public DataHolder<GatherPipelineGlobal, SlotID::GatherPipelineGlobal,Table::GatherPipelineGlobal,DefaultLayout::Instance0>
	{
		SRV srv;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance0::SRV_ID};
		GatherPipelineGlobal(): DataHolder(srv){}
		GatherPipelineGlobal(const GatherPipelineGlobal&other): DataHolder(srv){srv = other.srv;}
	};
}
