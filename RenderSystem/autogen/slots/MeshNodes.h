#pragma once
#include "..\Tables\MeshNodes.h"
namespace Slots {
	struct MeshNodes:public DataHolder<Table::MeshNodes,DefaultLayout::Instance2>
	{
		SRV srv;
		MeshNodes(): DataHolder(srv){}
	};
}
