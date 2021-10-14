#pragma once
#include "..\Tables\Test.h"
namespace Slots {
	struct Test:public DataHolder<Test, SlotID::Test,Table::Test,DefaultLayout::Instance0>
	{
		CB cb;
		Render::Bindless bindless;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance0::CB_ID};
		Test(): DataHolder(cb,bindless){}
		Test(const Test&other): DataHolder(cb,bindless){cb = other.cb;}
	};
}
