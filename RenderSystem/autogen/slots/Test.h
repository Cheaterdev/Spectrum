#pragma once
#include "..\Tables\Test.h"
namespace Slots {
	struct Test:public DataHolder<Table::Test,DefaultLayout::Instance0>
	{
		CB cb;
		Render::Bindless bindless;
		Test(): DataHolder(cb,bindless){}
	};
}
