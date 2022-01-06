#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FSR.h"
ConstantBuffer<FSR> pass_FSR: register( b2, space4);
const FSR CreateFSR()
{
	return pass_FSR;
}
#ifndef NO_GLOBAL
static const FSR fSR_global = CreateFSR();
const FSR GetFSR(){ return fSR_global; }
#endif
