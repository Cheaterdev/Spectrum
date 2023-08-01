export module HAL:Autogen.Slots.DrawBoxes;
import Core;
import :Autogen.Tables.DrawBoxes;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct DrawBoxes:public DataHolder<DrawBoxes, SlotID::DrawBoxes,Table::DrawBoxes,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DrawBoxes() = default;
	};
}
