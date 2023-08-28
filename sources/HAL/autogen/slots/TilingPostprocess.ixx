export module HAL:Autogen.Slots.TilingPostprocess;
import Core;
import :Autogen.Tables.TilingPostprocess;
import :Autogen.Layouts.FrameLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct TilingPostprocess:public DataHolder<TilingPostprocess, SlotID::TilingPostprocess,Table::TilingPostprocess,FrameLayout::PassData>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		TilingPostprocess() = default;
	};
}
