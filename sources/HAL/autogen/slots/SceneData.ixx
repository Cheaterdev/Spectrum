export module HAL:Autogen.Slots.SceneData;
import Core;
import :Autogen.Tables.SceneData;
import :Autogen.Layouts.FrameLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct SceneData:public DataHolder<SceneData, SlotID::SceneData,Table::SceneData,FrameLayout::SceneData>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		SceneData() = default;
	};
}
