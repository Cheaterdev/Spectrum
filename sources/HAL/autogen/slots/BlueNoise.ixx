export module HAL:Autogen.Slots.BlueNoise;
import Core;
import :Autogen.Tables.BlueNoise;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct BlueNoise:public DataHolder<BlueNoise, SlotID::BlueNoise,Table::BlueNoise,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		BlueNoise() = default;
	};
}
