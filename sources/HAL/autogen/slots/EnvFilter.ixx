export module HAL:Autogen.Slots.EnvFilter;
import Core;
import :Autogen.Tables.EnvFilter;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct EnvFilter:public DataHolder<EnvFilter, SlotID::EnvFilter,Table::EnvFilter,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		EnvFilter() = default;
	};
}
