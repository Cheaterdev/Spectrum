#pragma once
#include "..\Tables\Test.h"
namespace Slots {
	struct Test:public DataHolder<Table::Test,DefaultLayout::Instance0>
	{
		CB cb;
		DX12::Bindless bindless;
		Test(): DataHolder(cb,bindless){}
		Test(const Test&other): DataHolder(cb,bindless){cb = other.cb;}
	};
}
