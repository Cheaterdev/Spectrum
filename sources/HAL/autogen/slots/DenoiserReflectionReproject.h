#pragma once
#include "..\Tables\DenoiserReflectionReproject.h"
namespace Slots {
	struct DenoiserReflectionReproject:public DataHolder<DenoiserReflectionReproject, SlotID::DenoiserReflectionReproject,Table::DenoiserReflectionReproject,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DenoiserReflectionReproject() = default;
	};
}
