export module HAL:Autogen.Slots.ReflectionCombine;
import Core;
import :Autogen.Tables.ReflectionCombine;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct ReflectionCombine:public DataHolder<ReflectionCombine, SlotID::ReflectionCombine,Table::ReflectionCombine,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		ReflectionCombine() = default;
	};
}
