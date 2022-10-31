#include "pch_render.h"
#include "RTX.h"


void RTX::prepare(HAL::CommandList::ptr& list)
{
	rtx.prepare(list);
}
