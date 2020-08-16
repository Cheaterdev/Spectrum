#pragma once
#include "..\Tables\MeshData.h"
namespace Slots {
	struct MeshData:public DataHolder<Table::MeshData,DefaultLayout::Instance0>
	{
		SRV srv;
		MeshData(): DataHolder(srv){}
		MeshData(const MeshData&other): DataHolder(srv){srv = other.srv;}
	};
}
