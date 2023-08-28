export module HAL:Autogen.Slots.DenoiserReflectionPrefilter;
import Core;
import :Autogen.Tables.DenoiserReflectionPrefilter;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct DenoiserReflectionPrefilter:public DataHolder<DenoiserReflectionPrefilter, SlotID::DenoiserReflectionPrefilter,Table::DenoiserReflectionPrefilter,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DenoiserReflectionPrefilter() = default;
	};
}
