#pragma once
#include "..\Tables\DrawStencil.h"
namespace Slots {
	struct DrawStencil:public DataHolder<DrawStencil, SlotID::DrawStencil,Table::DrawStencil,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DrawStencil() = default;
	};
}
