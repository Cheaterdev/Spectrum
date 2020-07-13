#pragma once
#include "..\Tables\GatherMeshesBoxes.h"
namespace Slots {
	struct GatherMeshesBoxes:public DataHolder<Table::GatherMeshesBoxes,DefaultLayout::Instance1>
	{
		SRV srv;
		UAV uav;
		GatherMeshesBoxes(): DataHolder(srv,uav){}
		GatherMeshesBoxes(const GatherMeshesBoxes&other): DataHolder(srv,uav){srv = other.srv;uav = other.uav;}
	};
}
