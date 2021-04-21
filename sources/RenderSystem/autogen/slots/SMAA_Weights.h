#pragma once
#include "..\Tables\SMAA_Weights.h"
namespace Slots {
	struct SMAA_Weights:public DataHolder<SlotID::SMAA_Weights,Table::SMAA_Weights,DefaultLayout::Instance1>
	{
		SRV srv;
		SMAA_Weights(): DataHolder(srv){}
		SMAA_Weights(const SMAA_Weights&other): DataHolder(srv){srv = other.srv;}
	};
}
