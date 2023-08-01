export module HAL:Autogen.Slots.GatherPipelineGlobal;
import Core;
import :Autogen.Tables.GatherPipelineGlobal;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct GatherPipelineGlobal:public DataHolder<GatherPipelineGlobal, SlotID::GatherPipelineGlobal,Table::GatherPipelineGlobal,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		GatherPipelineGlobal() = default;
	};
}
