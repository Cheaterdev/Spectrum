#pragma once
#include "..\Tables\Raytracing.h"
namespace Slots {
	struct Raytracing:public DataHolder<Raytracing, SlotID::Raytracing,Table::Raytracing,DefaultLayout::Raytracing>
	{
		SRV srv;
		static inline const std::vector<UINT> tables = {DefaultLayout::Raytracing::SRV_ID};
		Raytracing(): DataHolder(srv){}
		Raytracing(const Raytracing&other): DataHolder(srv){srv = other.srv;}
	};
}
