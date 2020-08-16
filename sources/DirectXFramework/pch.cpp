#include "pch.h"

static std::array<DX12::RootLayout::ptr, static_cast<int>(Layouts::TOTAL)> signatures;
void init_signatures()
{
	signatures[static_cast<int>(Layouts::FrameLayout)] = AutoGenSignatureDesc<FrameLayout>().create_signature(Layouts::FrameLayout);
	signatures[static_cast<int>(Layouts::DefaultLayout)] = AutoGenSignatureDesc<DefaultLayout>().create_signature(Layouts::DefaultLayout);
}
DX12::RootLayout::ptr get_Signature(Layouts id)
{
	return signatures[static_cast<int>(id)];
}



