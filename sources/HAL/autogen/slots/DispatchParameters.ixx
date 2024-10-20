export module HAL:Autogen.Slots.DispatchParameters;
import Core;
import :Autogen.Tables.DispatchParameters;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;

export namespace Slots 
{
	struct DispatchParameters :public DataHolder<DispatchParameters, SlotID::DispatchParameters, Table::DispatchParameters, DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DispatchParameters() = default;
	};
}