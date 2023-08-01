export module HAL:Autogen.Slots.DenoiserReflectionReproject;
import Core;
import :Autogen.Tables.DenoiserReflectionReproject;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct DenoiserReflectionReproject:public DataHolder<DenoiserReflectionReproject, SlotID::DenoiserReflectionReproject,Table::DenoiserReflectionReproject,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DenoiserReflectionReproject() = default;
	};
}
