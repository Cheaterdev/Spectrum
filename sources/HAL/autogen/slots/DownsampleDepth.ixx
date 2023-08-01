export module HAL:Autogen.Slots.DownsampleDepth;
import Core;
import :Autogen.Tables.DownsampleDepth;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct DownsampleDepth:public DataHolder<DownsampleDepth, SlotID::DownsampleDepth,Table::DownsampleDepth,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DownsampleDepth() = default;
	};
}
