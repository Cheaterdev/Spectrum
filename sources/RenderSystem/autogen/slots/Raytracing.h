#pragma once
#include "..\Tables\Raytracing.h"
namespace Slots {
	struct Raytracing:public DataHolder<Table::Raytracing,DefaultLayout::Raytracing>
	{
		SRV srv;
		Raytracing(): DataHolder(srv){}
		Raytracing(const Raytracing&other): DataHolder(srv){srv = other.srv;}
	};
}
