#pragma once
#include "..\Tables\DenoiserShadow_TileClassification.h"
namespace Slots {
	struct DenoiserShadow_TileClassification:public DataHolder<DenoiserShadow_TileClassification, SlotID::DenoiserShadow_TileClassification,Table::DenoiserShadow_TileClassification,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DenoiserShadow_TileClassification() = default;
	};
}
