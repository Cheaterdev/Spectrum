export module HAL:Autogen.Slots.GatherPipeline;
import Core;
import :Autogen.Tables.GatherPipeline;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct GatherPipeline:public DataHolder<GatherPipeline, SlotID::GatherPipeline,Table::GatherPipeline,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		GatherPipeline() = default;
	};
}
