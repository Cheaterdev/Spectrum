#pragma once
#include "..\Tables\DenoiserReflectionResolve.h"
namespace Slots {
	struct DenoiserReflectionResolve:public DataHolder<DenoiserReflectionResolve, SlotID::DenoiserReflectionResolve,Table::DenoiserReflectionResolve,DefaultLayout::Instance1>
	{
		DenoiserReflectionResolve() = default;
	};
}
