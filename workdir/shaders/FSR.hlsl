#include "autogen/FSR.h"

static Texture2D<float4> InputTexture = GetFSR().GetSource();
static RWTexture2D<float4> OutputTexture = GetFSR().GetTarget();
static const uint4 Const0 = GetFSR().GetConstants().GetConst0();
static const uint4 Const1 = GetFSR().GetConstants().GetConst1();
static const uint4 Const2 = GetFSR().GetConstants().GetConst2();
static const uint4 Const3 = GetFSR().GetConstants().GetConst3();

#define CONCAT2(a, b) a ## b
#define CONCAT(a, b) CONCAT2(a, b)

#define A_GPU 1
#define A_HLSL 1
//#define A_HALF

#include "fsr/ffx_a.h"

#ifdef A_HALF
#define Type4 AH4
#define Type3 AH3
#define Type1 AH1

#ifdef RCAS
#define FSR_RCAS_H
#define FsrRcasInput FsrRcasInputH
Type4 FsrRcasLoadH(ASW2 p) { return InputTexture.Load(ASW3(ASW2(p), 0)); }
#define FsrRcas FsrRcasH
#else
#define FSR_EASU_H
#define FsrEasuR FsrEasuRH
#define FsrEasuG FsrEasuGH
#define FsrEasuB FsrEasuBH
#define FsrEasu FsrEasuH
#endif

#else
#define Type4 AF4
#define Type3 AF3
#define Type1 AF1



#ifdef RCAS
#define FSR_RCAS_F
#define FsrRcasInput FsrRcasInputF
Type4 FsrRcasLoadF(ASU2 p) { return InputTexture.Load(int3(ASU2(p), 0)); }
#define FsrRcas FsrRcasF
#else
#define FSR_EASU_F
#define FsrEasuR FsrEasuRF
#define FsrEasuG FsrEasuGF
#define FsrEasuB FsrEasuBF
#define FsrEasu FsrEasuF
#endif



#endif


#ifdef RCAS
void FsrRcasInput(inout Type1 r, inout Type1 g, inout Type1 b) {}
#else
Type4 FsrEasuR(AF2 p) { Type4 res = InputTexture.GatherRed(linearClampSampler, p, int2(0, 0)); return res; }
Type4 FsrEasuG(AF2 p) { Type4 res = InputTexture.GatherGreen(linearClampSampler, p, int2(0, 0)); return res; }
Type4 FsrEasuB(AF2 p) { Type4 res = InputTexture.GatherBlue(linearClampSampler, p, int2(0, 0)); return res; }
#endif

#include "fsr/ffx_fsr1.h"



void CurrFilter(int2 pos)
{
#ifdef RCAS
	Type3 c;
	FsrRcas(c.r, c.g, c.b, pos, Const0);
	OutputTexture[pos] = Type4(c, 1);
#else
	Type3 c;
	FsrEasu(c, pos, Const0, Const1, Const2, Const3);
	OutputTexture[pos] = Type4(c, 1);
#endif
	
}

[numthreads(64,1,1)]
void CS(uint3 LocalThreadId : SV_GroupThreadID, uint3 WorkGroupId : SV_GroupID, uint3 Dtid : SV_DispatchThreadID)
{
	// Do remapping of local xy in workgroup for a more PS-like swizzle pattern.
	AU2 gxy = ARmp8x8(LocalThreadId.x) + AU2(WorkGroupId.x << 4u, WorkGroupId.y << 4u);
	CurrFilter(gxy);
	gxy.x += 8u;
	CurrFilter(gxy);
	gxy.y += 8u;
	CurrFilter(gxy);
	gxy.x -= 8u;
	CurrFilter(gxy);
	
}

