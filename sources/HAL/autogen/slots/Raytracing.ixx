export module HAL:Autogen.Slots.Raytracing;
import Core;
import :Autogen.Tables.Raytracing;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct Raytracing:public DataHolder<Raytracing, SlotID::Raytracing,Table::Raytracing,DefaultLayout::Raytracing>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		Raytracing() = default;
	};
}
