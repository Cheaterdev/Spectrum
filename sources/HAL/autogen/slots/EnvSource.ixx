export module HAL:Autogen.Slots.EnvSource;
import Core;
import :Autogen.Tables.EnvSource;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct EnvSource:public DataHolder<EnvSource, SlotID::EnvSource,Table::EnvSource,DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		EnvSource() = default;
	};
}
