export module HAL:Autogen.Slots.DenoiserShadow_TileClassification;
import Core;
import :Autogen.Tables.DenoiserShadow_TileClassification;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct DenoiserShadow_TileClassification:public DataHolder<DenoiserShadow_TileClassification, SlotID::DenoiserShadow_TileClassification,Table::DenoiserShadow_TileClassification,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DenoiserShadow_TileClassification() = default;
	};
}
