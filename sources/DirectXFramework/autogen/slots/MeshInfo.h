#pragma once
#include "..\Tables\MeshInfo.h"
namespace Slots {
	struct MeshInfo:public DataHolder<MeshInfo, SlotID::MeshInfo,Table::MeshInfo,DefaultLayout::Instance1>
	{
		CB cb;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance1::CB_ID};
		MeshInfo(): DataHolder(cb){}
		MeshInfo(const MeshInfo&other): DataHolder(cb){cb = other.cb;}
	};
}
