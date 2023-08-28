export module HAL:Autogen.Slots.RaytracingRays;
import Core;
import :Autogen.Tables.RaytracingRays;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct RaytracingRays:public DataHolder<RaytracingRays, SlotID::RaytracingRays,Table::RaytracingRays,DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		RaytracingRays() = default;
	};
}
