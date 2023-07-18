#pragma once
#include "..\Tables\DenoiserReflectionResolve.h"
namespace Slots {
	struct DenoiserReflectionResolve:public DataHolder<DenoiserReflectionResolve, SlotID::DenoiserReflectionResolve,Table::DenoiserReflectionResolve,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DenoiserReflectionResolve() = default;
	};
}
