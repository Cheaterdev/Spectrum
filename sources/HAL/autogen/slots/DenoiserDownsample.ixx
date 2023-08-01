export module HAL:Autogen.Slots.DenoiserDownsample;
import Core;
import :Autogen.Tables.DenoiserDownsample;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct DenoiserDownsample:public DataHolder<DenoiserDownsample, SlotID::DenoiserDownsample,Table::DenoiserDownsample,DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DenoiserDownsample() = default;
	};
}
