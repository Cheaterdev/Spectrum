#pragma once
#include "..\Tables\CopyTexture.h"
namespace Slots {
	struct CopyTexture:public DataHolder<CopyTexture, SlotID::CopyTexture,Table::CopyTexture,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		CopyTexture() = default;
	};
}
