export module HAL:Autogen.Slots.SkyFace;
import Core;
import :Autogen.Tables.SkyFace;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct SkyFace:public DataHolder<SkyFace, SlotID::SkyFace,Table::SkyFace,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		SkyFace() = default;
	};
}
