export module HAL:Autogen.Slots.BRDF;
import Core;
import :Autogen.Tables.BRDF;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct BRDF:public DataHolder<BRDF, SlotID::BRDF,Table::BRDF,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		BRDF() = default;
	};
}
