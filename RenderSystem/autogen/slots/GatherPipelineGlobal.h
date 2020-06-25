#pragma once
#include "..\Tables\GatherPipelineGlobal.h"
namespace Slots {
	struct GatherPipelineGlobal:public DataHolder<Table::GatherPipelineGlobal,DefaultLayout::Instance0>
	{
		CB cb;
		SRV srv;
		GatherPipelineGlobal(): DataHolder(cb,srv){}
		GatherPipelineGlobal(const GatherPipelineGlobal&other): DataHolder(cb,srv){cb = other.cb;srv = other.srv;}
	};
}
