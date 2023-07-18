#pragma once
#include "..\Tables\DenoiserReflectionPrefilter.h"
namespace Slots {
	struct DenoiserReflectionPrefilter:public DataHolder<DenoiserReflectionPrefilter, SlotID::DenoiserReflectionPrefilter,Table::DenoiserReflectionPrefilter,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DenoiserReflectionPrefilter() = default;
	};
}
