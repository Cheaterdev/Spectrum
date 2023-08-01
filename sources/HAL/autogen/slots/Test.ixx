export module HAL:Autogen.Slots.Test;
import Core;
import :Autogen.Tables.Test;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct Test:public DataHolder<Test, SlotID::Test,Table::Test,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		Test() = default;
	};
}
