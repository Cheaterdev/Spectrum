export module HAL:Autogen.Slots.DenoiserReflectionResolve;
import Core;
import :Autogen.Tables.DenoiserReflectionResolve;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct DenoiserReflectionResolve:public DataHolder<DenoiserReflectionResolve, SlotID::DenoiserReflectionResolve,Table::DenoiserReflectionResolve,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DenoiserReflectionResolve() = default;
	};
}
