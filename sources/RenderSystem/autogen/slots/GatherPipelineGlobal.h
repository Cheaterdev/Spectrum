#pragma once
#include "..\Tables\GatherPipelineGlobal.h"
namespace Slots {
	struct GatherPipelineGlobal:public DataHolder<SlotID::GatherPipelineGlobal,Table::GatherPipelineGlobal,DefaultLayout::Instance0>
	{
		SRV srv;
		GatherPipelineGlobal(): DataHolder(srv){}
		GatherPipelineGlobal(const GatherPipelineGlobal&other): DataHolder(srv){srv = other.srv;}
	};
}
