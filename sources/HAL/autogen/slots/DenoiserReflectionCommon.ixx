export module HAL:Autogen.Slots.DenoiserReflectionCommon;
import Core;
import :Autogen.Tables.DenoiserReflectionCommon;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct DenoiserReflectionCommon:public DataHolder<DenoiserReflectionCommon, SlotID::DenoiserReflectionCommon,Table::DenoiserReflectionCommon,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DenoiserReflectionCommon() = default;
	};
}
