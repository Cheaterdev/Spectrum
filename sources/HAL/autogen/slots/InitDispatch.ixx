export module HAL:Autogen.Slots.InitDispatch;
import Core;
import :Autogen.Tables.InitDispatch;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct InitDispatch:public DataHolder<InitDispatch, SlotID::InitDispatch,Table::InitDispatch,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		InitDispatch() = default;
	};
}
