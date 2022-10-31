#pragma once
#include "..\Tables\Test.h"
namespace Slots {
	struct Test:public DataHolder<Test, SlotID::Test,Table::Test,DefaultLayout::Instance0>
	{
		Test() = default;
	};
}
