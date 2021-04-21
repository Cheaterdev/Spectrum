#pragma once
#include "..\Tables\Test.h"
namespace Slots {
	struct Test:public DataHolder<SlotID::Test,Table::Test,DefaultLayout::Instance0>
	{
		CB cb;
		Render::Bindless bindless;
		Test(): DataHolder(cb,bindless){}
		Test(const Test&other): DataHolder(cb,bindless){cb = other.cb;}
	};
}
