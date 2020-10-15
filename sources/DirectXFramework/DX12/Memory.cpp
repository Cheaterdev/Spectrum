#include "pch.h"

void DX12::ResourceHandle::Free()
{
	if (!handle) return;
	owner->free(handle);
}
