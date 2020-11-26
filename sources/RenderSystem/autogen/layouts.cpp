#include "pch.h"
static enum_array<Layouts, DX12::RootLayout::ptr> signatures;
void init_signatures()
{
	signatures[Layouts::FrameLayout] = AutoGenSignatureDesc<FrameLayout>().create_signature(Layouts::FrameLayout);
	signatures[Layouts::DefaultLayout] = AutoGenSignatureDesc<DefaultLayout>().create_signature(Layouts::DefaultLayout);
}
Render::RootLayout::ptr get_Signature(Layouts id)
{
	return signatures[id];
}
