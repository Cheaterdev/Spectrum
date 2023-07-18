#pragma once
#include "..\Tables\InitDispatch.h"
namespace Slots {
	struct InitDispatch:public DataHolder<InitDispatch, SlotID::InitDispatch,Table::InitDispatch,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		InitDispatch() = default;
	};
}
