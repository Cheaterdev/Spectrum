#pragma once
#include "..\Tables\GatherMeshesBoxes.h"
namespace Slots {
	struct GatherMeshesBoxes:public DataHolder<GatherMeshesBoxes, SlotID::GatherMeshesBoxes,Table::GatherMeshesBoxes,DefaultLayout::Instance1>
	{
		SRV srv;
		UAV uav;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance1::SRV_ID, DefaultLayout::Instance1::UAV_ID};
		GatherMeshesBoxes(): DataHolder(srv,uav){}
		GatherMeshesBoxes(const GatherMeshesBoxes&other): DataHolder(srv,uav){srv = other.srv;uav = other.uav;}
	};
}
