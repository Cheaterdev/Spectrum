#include "autogen/bobo.h"
//#include "autogen/bobo2.h"
//#include "autogen/bobo3.h"


static const float x = 4;

struct quad_output
{
	float4 pos : SV_POSITION;
	float2 tc : TEXCOORD0;
};

static const Bobo mybobo = bobo_global;//GetBobo();

float4 PS(quad_output i): SV_Target0
{
    return GetBobo().GetTessst().GetCurrent().GetPos();
}
