#pragma once
#include "..\Tables\NinePatch.h"
namespace Slots {
	struct NinePatch:public DataHolder<NinePatch, SlotID::NinePatch,Table::NinePatch,DefaultLayout::Instance0>
	{
		SRV srv;
		Render::Bindless bindless;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance0::SRV_ID};
		NinePatch(): DataHolder(srv,bindless){}
		NinePatch(const NinePatch&other): DataHolder(srv,bindless){srv = other.srv;}
	};
}
