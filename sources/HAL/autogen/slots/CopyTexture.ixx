export module HAL:Autogen.Slots.CopyTexture;
import Core;
import :Autogen.Tables.CopyTexture;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct CopyTexture:public DataHolder<CopyTexture, SlotID::CopyTexture,Table::CopyTexture,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		CopyTexture() = default;
	};
}
