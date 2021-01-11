#include "pch.h"
#include "DX12/RootSignature.h"
#include <slots.h>
static enum_array<Layouts, DX12::RootLayout::ptr> signatures;
void init_signatures()
{
	signatures[Layouts::FrameLayout] = AutoGenSignatureDesc<FrameLayout>().create_signature(Layouts::FrameLayout);
	signatures[Layouts::DefaultLayout] = AutoGenSignatureDesc<DefaultLayout>().create_signature(Layouts::DefaultLayout);
}
DX12::RootLayout::ptr get_Signature(Layouts id)
{
	return signatures[id];
}
