#pragma once
#include "..\Tables\MeshInfo.h"
namespace Slots {
	struct MeshInfo:public DataHolder<SlotID::MeshInfo,Table::MeshInfo,DefaultLayout::Instance1>
	{
		CB cb;
		MeshInfo(): DataHolder(cb){}
		MeshInfo(const MeshInfo&other): DataHolder(cb){cb = other.cb;}
	};
}
