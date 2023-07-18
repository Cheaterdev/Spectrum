#pragma once
#include "..\Tables\GBuffer.h"
namespace Slots {
	struct GBuffer:public DataHolder<GBuffer, SlotID::GBuffer,Table::GBuffer,DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		GBuffer() = default;
	};
}
