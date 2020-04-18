#pragma once
#include "..\Tables\Material.h"
namespace Slots {
	struct Material:public DataHolder<Table::Material,FrameLayout::MaterialData>
	{
		CB cb;
		Material(): DataHolder(cb){}
	};
}
