export module HAL:Autogen.Slots.Instance;
import Core;
import :Autogen.Tables.Instance;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct Instance:public DataHolder<Instance, SlotID::Instance,Table::Instance,DefaultLayout::Instance3>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		Instance() = default;
	};
}
