#pragma once
#include "..\Tables\DenoiserReflectionPrefilter.h"
namespace Slots {
	struct DenoiserReflectionPrefilter:public DataHolder<DenoiserReflectionPrefilter, SlotID::DenoiserReflectionPrefilter,Table::DenoiserReflectionPrefilter,DefaultLayout::Instance1>
	{
		DenoiserReflectionPrefilter() = default;
	};
}
