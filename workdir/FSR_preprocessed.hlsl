#line 1 "C:\\github\\Spectrum\\workdir\\shaders\\FSR.hlsl"
#line 1 "C:\\github\\Spectrum\\workdir\\shaders/autogen/FSR.h"
#line 13 "C:\\github\\Spectrum\\workdir\\shaders/autogen/FSR.h"
#line 1 "C:\\github\\Spectrum\\workdir\\shaders/autogen/layout/DefaultLayout.h"







#line 1 "C:\\github\\Spectrum\\workdir\\shaders/autogen/layout/FrameLayout.h"







SamplerState linearSampler:register(s0);
SamplerState pointClampSampler:register(s1);
SamplerState linearClampSampler:register(s2);
SamplerState anisoBordeSampler:register(s3);
SamplerState pointBorderSampler:register(s4);
#line 8 "C:\\github\\Spectrum\\workdir\\shaders/autogen/layout/DefaultLayout.h"
#line 14 "C:\\github\\Spectrum\\workdir\\shaders/autogen/FSR.h"
#line 1 "C:\\github\\Spectrum\\workdir\\shaders/autogen/tables/FSR.h"







#line 1 "C:\\github\\Spectrum\\workdir\\shaders/sig_hlsl.hlsl"
#line 8 "C:\\github\\Spectrum\\workdir\\shaders/autogen/tables/FSR.h"

#line 1 "C:\\github\\Spectrum\\workdir\\shaders/autogen/tables/FSRConstants.h"
#line 12 "C:\\github\\Spectrum\\workdir\\shaders/autogen/tables/FSRConstants.h"
struct FSRConstants
{
 uint4 Const0;
 uint4 Const1;
 uint4 Const2;
 uint4 Const3;
 uint4 Sample;
 uint4 GetConst0() { return Const0; }
 uint4 GetConst1() { return Const1; }
 uint4 GetConst2() { return Const2; }
 uint4 GetConst3() { return Const3; }
 uint4 GetSample() { return Sample; }
};
#line 9 "C:\\github\\Spectrum\\workdir\\shaders/autogen/tables/FSR.h"




struct FSR
{
 uint source;
 uint target;
 FSRConstants constants;
 FSRConstants GetConstants() { return constants; }
 Texture2D<float4> GetSource() { return ResourceDescriptorHeap[source]; }
 RWTexture2D<float4> GetTarget() { return ResourceDescriptorHeap[target]; }
};
#line 15 "C:\\github\\Spectrum\\workdir\\shaders/autogen/FSR.h"



struct CB { uint offset; };






ConstantBuffer<CB> pass_FSR: register(b4, space4);


ConstantBuffer<FSR> CreateFSR()
{
 return ResourceDescriptorHeap[pass_FSR.offset];
}


static const ConstantBuffer<FSR> fSR_global = CreateFSR();
ConstantBuffer<FSR> GetFSR(){ return fSR_global; }
#line 1 "C:\\github\\Spectrum\\workdir\\shaders\\FSR.hlsl"


static Texture2D<float4> InputTexture = GetFSR().GetSource();
static RWTexture2D<float4> OutputTexture = GetFSR().GetTarget();
static const uint4 Const0 = GetFSR().GetConstants().GetConst0();
static const uint4 Const1 = GetFSR().GetConstants().GetConst1();
static const uint4 Const2 = GetFSR().GetConstants().GetConst2();
static const uint4 Const3 = GetFSR().GetConstants().GetConst3();
#line 17 "C:\\github\\Spectrum\\workdir\\shaders\\FSR.hlsl"
#line 1 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_a.h"
#line 1089 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_a.h"
 uint AU1_AH1_AF1_x(float a){return f32tof16(a);}


 uint AU1_AH2_AF2_x(float2 a){return f32tof16(a.x)|(f32tof16(a.y)<<16);}



 float2 AF2_AH2_AU1_x(uint x){return float2(f16tof32(x&0xFFFF),f16tof32(x>>16));}


 float AF1_x(float a){return float(a);}
 float2 AF2_x(float a){return float2(a,a);}
 float3 AF3_x(float a){return float3(a,a,a);}
 float4 AF4_x(float a){return float4(a,a,a,a);}





 uint AU1_x(uint a){return uint(a);}
 uint2 AU2_x(uint a){return uint2(a,a);}
 uint3 AU3_x(uint a){return uint3(a,a,a);}
 uint4 AU4_x(uint a){return uint4(a,a,a,a);}





 uint AAbsSU1(uint a){return uint(abs(int(a)));}
 uint2 AAbsSU2(uint2 a){return uint2(abs(int2(a)));}
 uint3 AAbsSU3(uint3 a){return uint3(abs(int3(a)));}
 uint4 AAbsSU4(uint4 a){return uint4(abs(int4(a)));}

 uint ABfe(uint src,uint off,uint bits){uint mask=(1u<<bits)-1;return (src>>off)&mask;}
 uint ABfi(uint src,uint ins,uint mask){return (ins&mask)|(src&(~mask));}
 uint ABfiM(uint src,uint ins,uint bits){uint mask=(1u<<bits)-1;return (ins&mask)|(src&(~mask));}

 float AClampF1(float x,float n,float m){return max(n,min(x,m));}
 float2 AClampF2(float2 x,float2 n,float2 m){return max(n,min(x,m));}
 float3 AClampF3(float3 x,float3 n,float3 m){return max(n,min(x,m));}
 float4 AClampF4(float4 x,float4 n,float4 m){return max(n,min(x,m));}

 float AFractF1(float x){return x-floor(x);}
 float2 AFractF2(float2 x){return x-floor(x);}
 float3 AFractF3(float3 x){return x-floor(x);}
 float4 AFractF4(float4 x){return x-floor(x);}

 float ALerpF1(float x,float y,float a){return lerp(x,y,a);}
 float2 ALerpF2(float2 x,float2 y,float2 a){return lerp(x,y,a);}
 float3 ALerpF3(float3 x,float3 y,float3 a){return lerp(x,y,a);}
 float4 ALerpF4(float4 x,float4 y,float4 a){return lerp(x,y,a);}

 float AMax3F1(float x,float y,float z){return max(x,max(y,z));}
 float2 AMax3F2(float2 x,float2 y,float2 z){return max(x,max(y,z));}
 float3 AMax3F3(float3 x,float3 y,float3 z){return max(x,max(y,z));}
 float4 AMax3F4(float4 x,float4 y,float4 z){return max(x,max(y,z));}

 uint AMax3SU1(uint x,uint y,uint z){return uint(max(int(x),max(int(y),int(z))));}
 uint2 AMax3SU2(uint2 x,uint2 y,uint2 z){return uint2(max(int2(x),max(int2(y),int2(z))));}
 uint3 AMax3SU3(uint3 x,uint3 y,uint3 z){return uint3(max(int3(x),max(int3(y),int3(z))));}
 uint4 AMax3SU4(uint4 x,uint4 y,uint4 z){return uint4(max(int4(x),max(int4(y),int4(z))));}

 uint AMax3U1(uint x,uint y,uint z){return max(x,max(y,z));}
 uint2 AMax3U2(uint2 x,uint2 y,uint2 z){return max(x,max(y,z));}
 uint3 AMax3U3(uint3 x,uint3 y,uint3 z){return max(x,max(y,z));}
 uint4 AMax3U4(uint4 x,uint4 y,uint4 z){return max(x,max(y,z));}

 uint AMaxSU1(uint a,uint b){return uint(max(int(a),int(b)));}
 uint2 AMaxSU2(uint2 a,uint2 b){return uint2(max(int2(a),int2(b)));}
 uint3 AMaxSU3(uint3 a,uint3 b){return uint3(max(int3(a),int3(b)));}
 uint4 AMaxSU4(uint4 a,uint4 b){return uint4(max(int4(a),int4(b)));}

 float AMed3F1(float x,float y,float z){return max(min(x,y),min(max(x,y),z));}
 float2 AMed3F2(float2 x,float2 y,float2 z){return max(min(x,y),min(max(x,y),z));}
 float3 AMed3F3(float3 x,float3 y,float3 z){return max(min(x,y),min(max(x,y),z));}
 float4 AMed3F4(float4 x,float4 y,float4 z){return max(min(x,y),min(max(x,y),z));}

 float AMin3F1(float x,float y,float z){return min(x,min(y,z));}
 float2 AMin3F2(float2 x,float2 y,float2 z){return min(x,min(y,z));}
 float3 AMin3F3(float3 x,float3 y,float3 z){return min(x,min(y,z));}
 float4 AMin3F4(float4 x,float4 y,float4 z){return min(x,min(y,z));}

 uint AMin3SU1(uint x,uint y,uint z){return uint(min(int(x),min(int(y),int(z))));}
 uint2 AMin3SU2(uint2 x,uint2 y,uint2 z){return uint2(min(int2(x),min(int2(y),int2(z))));}
 uint3 AMin3SU3(uint3 x,uint3 y,uint3 z){return uint3(min(int3(x),min(int3(y),int3(z))));}
 uint4 AMin3SU4(uint4 x,uint4 y,uint4 z){return uint4(min(int4(x),min(int4(y),int4(z))));}

 uint AMin3U1(uint x,uint y,uint z){return min(x,min(y,z));}
 uint2 AMin3U2(uint2 x,uint2 y,uint2 z){return min(x,min(y,z));}
 uint3 AMin3U3(uint3 x,uint3 y,uint3 z){return min(x,min(y,z));}
 uint4 AMin3U4(uint4 x,uint4 y,uint4 z){return min(x,min(y,z));}

 uint AMinSU1(uint a,uint b){return uint(min(int(a),int(b)));}
 uint2 AMinSU2(uint2 a,uint2 b){return uint2(min(int2(a),int2(b)));}
 uint3 AMinSU3(uint3 a,uint3 b){return uint3(min(int3(a),int3(b)));}
 uint4 AMinSU4(uint4 a,uint4 b){return uint4(min(int4(a),int4(b)));}

 float ANCosF1(float x){return cos(x*AF1_x(float(6.28318530718)));}
 float2 ANCosF2(float2 x){return cos(x*AF2_x(float(6.28318530718)));}
 float3 ANCosF3(float3 x){return cos(x*AF3_x(float(6.28318530718)));}
 float4 ANCosF4(float4 x){return cos(x*AF4_x(float(6.28318530718)));}

 float ANSinF1(float x){return sin(x*AF1_x(float(6.28318530718)));}
 float2 ANSinF2(float2 x){return sin(x*AF2_x(float(6.28318530718)));}
 float3 ANSinF3(float3 x){return sin(x*AF3_x(float(6.28318530718)));}
 float4 ANSinF4(float4 x){return sin(x*AF4_x(float(6.28318530718)));}

 float ARcpF1(float x){return rcp(x);}
 float2 ARcpF2(float2 x){return rcp(x);}
 float3 ARcpF3(float3 x){return rcp(x);}
 float4 ARcpF4(float4 x){return rcp(x);}

 float ARsqF1(float x){return rsqrt(x);}
 float2 ARsqF2(float2 x){return rsqrt(x);}
 float3 ARsqF3(float3 x){return rsqrt(x);}
 float4 ARsqF4(float4 x){return rsqrt(x);}

 float ASatF1(float x){return saturate(x);}
 float2 ASatF2(float2 x){return saturate(x);}
 float3 ASatF3(float3 x){return saturate(x);}
 float4 ASatF4(float4 x){return saturate(x);}

 uint AShrSU1(uint a,uint b){return uint(int(a)>>int(b));}
 uint2 AShrSU2(uint2 a,uint2 b){return uint2(int2(a)>>int2(b));}
 uint3 AShrSU3(uint3 a,uint3 b){return uint3(int3(a)>>int3(b));}
 uint4 AShrSU4(uint4 a,uint4 b){return uint4(int4(a)>>int4(b));}
#line 1481 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_a.h"
 float ACpySgnF1(float d,float s){return asfloat(uint(asuint(float(d))|(asuint(float(s))&AU1_x(uint(0x80000000u)))));}
 float2 ACpySgnF2(float2 d,float2 s){return asfloat(uint2(asuint(float2(d))|(asuint(float2(s))&AU2_x(uint(0x80000000u)))));}
 float3 ACpySgnF3(float3 d,float3 s){return asfloat(uint3(asuint(float3(d))|(asuint(float3(s))&AU3_x(uint(0x80000000u)))));}
 float4 ACpySgnF4(float4 d,float4 s){return asfloat(uint4(asuint(float4(d))|(asuint(float4(s))&AU4_x(uint(0x80000000u)))));}
#line 1494 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_a.h"
 float ASignedF1(float m){return ASatF1(m*AF1_x(float(asfloat(uint(0xff800000u)))));}
 float2 ASignedF2(float2 m){return ASatF2(m*AF2_x(float(asfloat(uint(0xff800000u)))));}
 float3 ASignedF3(float3 m){return ASatF3(m*AF3_x(float(asfloat(uint(0xff800000u)))));}
 float4 ASignedF4(float4 m){return ASatF4(m*AF4_x(float(asfloat(uint(0xff800000u)))));}

 float AGtZeroF1(float m){return ASatF1(m*AF1_x(float(asfloat(uint(0x7f800000u)))));}
 float2 AGtZeroF2(float2 m){return ASatF2(m*AF2_x(float(asfloat(uint(0x7f800000u)))));}
 float3 AGtZeroF3(float3 m){return ASatF3(m*AF3_x(float(asfloat(uint(0x7f800000u)))));}
 float4 AGtZeroF4(float4 m){return ASatF4(m*AF4_x(float(asfloat(uint(0x7f800000u)))));}
#line 1546 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_a.h"
 uint AFisToU1(uint x){return x^(( AShrSU1(x,AU1_x(uint(31))))|AU1_x(uint(0x80000000)));}
 uint AFisFromU1(uint x){return x^((~AShrSU1(x,AU1_x(uint(31))))|AU1_x(uint(0x80000000)));}


 uint AFisToHiU1(uint x){return x^(( AShrSU1(x,AU1_x(uint(15))))|AU1_x(uint(0x80000000)));}
 uint AFisFromHiU1(uint x){return x^((~AShrSU1(x,AU1_x(uint(15))))|AU1_x(uint(0x80000000)));}
#line 1660 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_a.h"
  uint ABuc0ToU1(uint d,float i){return (d&0xffffff00u)|((min(uint(i),255u) )&(0x000000ffu));}
  uint ABuc1ToU1(uint d,float i){return (d&0xffff00ffu)|((min(uint(i),255u)<< 8)&(0x0000ff00u));}
  uint ABuc2ToU1(uint d,float i){return (d&0xff00ffffu)|((min(uint(i),255u)<<16)&(0x00ff0000u));}
  uint ABuc3ToU1(uint d,float i){return (d&0x00ffffffu)|((min(uint(i),255u)<<24)&(0xff000000u));}


  float ABuc0FromU1(uint i){return float((i )&255u);}
  float ABuc1FromU1(uint i){return float((i>> 8)&255u);}
  float ABuc2FromU1(uint i){return float((i>>16)&255u);}
  float ABuc3FromU1(uint i){return float((i>>24)&255u);}
#line 1728 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_a.h"
  uint ABsc0ToU1(uint d,float i){return (d&0xffffff00u)|((min(uint(i+128.0),255u) )&(0x000000ffu));}
  uint ABsc1ToU1(uint d,float i){return (d&0xffff00ffu)|((min(uint(i+128.0),255u)<< 8)&(0x0000ff00u));}
  uint ABsc2ToU1(uint d,float i){return (d&0xff00ffffu)|((min(uint(i+128.0),255u)<<16)&(0x00ff0000u));}
  uint ABsc3ToU1(uint d,float i){return (d&0x00ffffffu)|((min(uint(i+128.0),255u)<<24)&(0xff000000u));}

  uint ABsc0ToZbU1(uint d,float i){return ((d&0xffffff00u)|((min(uint(trunc(i)+128.0),255u) )&(0x000000ffu)))^0x00000080u;}
  uint ABsc1ToZbU1(uint d,float i){return ((d&0xffff00ffu)|((min(uint(trunc(i)+128.0),255u)<< 8)&(0x0000ff00u)))^0x00008000u;}
  uint ABsc2ToZbU1(uint d,float i){return ((d&0xff00ffffu)|((min(uint(trunc(i)+128.0),255u)<<16)&(0x00ff0000u)))^0x00800000u;}
  uint ABsc3ToZbU1(uint d,float i){return ((d&0x00ffffffu)|((min(uint(trunc(i)+128.0),255u)<<24)&(0xff000000u)))^0x80000000u;}

  float ABsc0FromU1(uint i){return float((i )&255u)-128.0;}
  float ABsc1FromU1(uint i){return float((i>> 8)&255u)-128.0;}
  float ABsc2FromU1(uint i){return float((i>>16)&255u)-128.0;}
  float ABsc3FromU1(uint i){return float((i>>24)&255u)-128.0;}

  float ABsc0FromZbU1(uint i){return float(((i )&255u)^0x80u)-128.0;}
  float ABsc1FromZbU1(uint i){return float(((i>> 8)&255u)^0x80u)-128.0;}
  float ABsc2FromZbU1(uint i){return float(((i>>16)&255u)^0x80u)-128.0;}
  float ABsc3FromZbU1(uint i){return float(((i>>24)&255u)^0x80u)-128.0;}
#line 1842 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_a.h"
 float APrxLoSqrtF1(float a){return asfloat(uint((asuint(float(a))>>AU1_x(uint(1)))+AU1_x(uint(0x1fbc4639))));}
 float APrxLoRcpF1(float a){return asfloat(uint(AU1_x(uint(0x7ef07ebb))-asuint(float(a))));}
 float APrxMedRcpF1(float a){float b=asfloat(uint(AU1_x(uint(0x7ef19fff))-asuint(float(a))));return b*(-b*a+AF1_x(float(2.0)));}
 float APrxLoRsqF1(float a){return asfloat(uint(AU1_x(uint(0x5f347d74))-(asuint(float(a))>>AU1_x(uint(1)))));}

 float2 APrxLoSqrtF2(float2 a){return asfloat(uint2((asuint(float2(a))>>AU2_x(uint(1)))+AU2_x(uint(0x1fbc4639))));}
 float2 APrxLoRcpF2(float2 a){return asfloat(uint2(AU2_x(uint(0x7ef07ebb))-asuint(float2(a))));}
 float2 APrxMedRcpF2(float2 a){float2 b=asfloat(uint2(AU2_x(uint(0x7ef19fff))-asuint(float2(a))));return b*(-b*a+AF2_x(float(2.0)));}
 float2 APrxLoRsqF2(float2 a){return asfloat(uint2(AU2_x(uint(0x5f347d74))-(asuint(float2(a))>>AU2_x(uint(1)))));}

 float3 APrxLoSqrtF3(float3 a){return asfloat(uint3((asuint(float3(a))>>AU3_x(uint(1)))+AU3_x(uint(0x1fbc4639))));}
 float3 APrxLoRcpF3(float3 a){return asfloat(uint3(AU3_x(uint(0x7ef07ebb))-asuint(float3(a))));}
 float3 APrxMedRcpF3(float3 a){float3 b=asfloat(uint3(AU3_x(uint(0x7ef19fff))-asuint(float3(a))));return b*(-b*a+AF3_x(float(2.0)));}
 float3 APrxLoRsqF3(float3 a){return asfloat(uint3(AU3_x(uint(0x5f347d74))-(asuint(float3(a))>>AU3_x(uint(1)))));}

 float4 APrxLoSqrtF4(float4 a){return asfloat(uint4((asuint(float4(a))>>AU4_x(uint(1)))+AU4_x(uint(0x1fbc4639))));}
 float4 APrxLoRcpF4(float4 a){return asfloat(uint4(AU4_x(uint(0x7ef07ebb))-asuint(float4(a))));}
 float4 APrxMedRcpF4(float4 a){float4 b=asfloat(uint4(AU4_x(uint(0x7ef19fff))-asuint(float4(a))));return b*(-b*a+AF4_x(float(2.0)));}
 float4 APrxLoRsqF4(float4 a){return asfloat(uint4(AU4_x(uint(0x5f347d74))-(asuint(float4(a))>>AU4_x(uint(1)))));}
#line 1871 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_a.h"
 float Quart(float a) { a = a * a; return a * a;}
 float Oct(float a) { a = a * a; a = a * a; return a * a; }
 float2 Quart(float2 a) { a = a * a; return a * a; }
 float2 Oct(float2 a) { a = a * a; a = a * a; return a * a; }
 float3 Quart(float3 a) { a = a * a; return a * a; }
 float3 Oct(float3 a) { a = a * a; a = a * a; return a * a; }
 float4 Quart(float4 a) { a = a * a; return a * a; }
 float4 Oct(float4 a) { a = a * a; a = a * a; return a * a; }

 float APrxPQToGamma2(float a) { return Quart(a); }
 float APrxPQToLinear(float a) { return Oct(a); }
 float APrxLoGamma2ToPQ(float a) { return asfloat(uint((asuint(float(a)) >> AU1_x(uint(2))) + AU1_x(uint(0x2F9A4E46)))); }
 float APrxMedGamma2ToPQ(float a) { float b = asfloat(uint((asuint(float(a)) >> AU1_x(uint(2))) + AU1_x(uint(0x2F9A4E46)))); float b4 = Quart(b); return b - b * (b4 - a) / (AF1_x(float(4.0)) * b4); }
 float APrxHighGamma2ToPQ(float a) { return sqrt(sqrt(a)); }
 float APrxLoLinearToPQ(float a) { return asfloat(uint((asuint(float(a)) >> AU1_x(uint(3))) + AU1_x(uint(0x378D8723)))); }
 float APrxMedLinearToPQ(float a) { float b = asfloat(uint((asuint(float(a)) >> AU1_x(uint(3))) + AU1_x(uint(0x378D8723)))); float b8 = Oct(b); return b - b * (b8 - a) / (AF1_x(float(8.0)) * b8); }
 float APrxHighLinearToPQ(float a) { return sqrt(sqrt(sqrt(a))); }

 float2 APrxPQToGamma2(float2 a) { return Quart(a); }
 float2 APrxPQToLinear(float2 a) { return Oct(a); }
 float2 APrxLoGamma2ToPQ(float2 a) { return asfloat(uint2((asuint(float2(a)) >> AU2_x(uint(2))) + AU2_x(uint(0x2F9A4E46)))); }
 float2 APrxMedGamma2ToPQ(float2 a) { float2 b = asfloat(uint2((asuint(float2(a)) >> AU2_x(uint(2))) + AU2_x(uint(0x2F9A4E46)))); float2 b4 = Quart(b); return b - b * (b4 - a) / (AF1_x(float(4.0)) * b4); }
 float2 APrxHighGamma2ToPQ(float2 a) { return sqrt(sqrt(a)); }
 float2 APrxLoLinearToPQ(float2 a) { return asfloat(uint2((asuint(float2(a)) >> AU2_x(uint(3))) + AU2_x(uint(0x378D8723)))); }
 float2 APrxMedLinearToPQ(float2 a) { float2 b = asfloat(uint2((asuint(float2(a)) >> AU2_x(uint(3))) + AU2_x(uint(0x378D8723)))); float2 b8 = Oct(b); return b - b * (b8 - a) / (AF1_x(float(8.0)) * b8); }
 float2 APrxHighLinearToPQ(float2 a) { return sqrt(sqrt(sqrt(a))); }

 float3 APrxPQToGamma2(float3 a) { return Quart(a); }
 float3 APrxPQToLinear(float3 a) { return Oct(a); }
 float3 APrxLoGamma2ToPQ(float3 a) { return asfloat(uint3((asuint(float3(a)) >> AU3_x(uint(2))) + AU3_x(uint(0x2F9A4E46)))); }
 float3 APrxMedGamma2ToPQ(float3 a) { float3 b = asfloat(uint3((asuint(float3(a)) >> AU3_x(uint(2))) + AU3_x(uint(0x2F9A4E46)))); float3 b4 = Quart(b); return b - b * (b4 - a) / (AF1_x(float(4.0)) * b4); }
 float3 APrxHighGamma2ToPQ(float3 a) { return sqrt(sqrt(a)); }
 float3 APrxLoLinearToPQ(float3 a) { return asfloat(uint3((asuint(float3(a)) >> AU3_x(uint(3))) + AU3_x(uint(0x378D8723)))); }
 float3 APrxMedLinearToPQ(float3 a) { float3 b = asfloat(uint3((asuint(float3(a)) >> AU3_x(uint(3))) + AU3_x(uint(0x378D8723)))); float3 b8 = Oct(b); return b - b * (b8 - a) / (AF1_x(float(8.0)) * b8); }
 float3 APrxHighLinearToPQ(float3 a) { return sqrt(sqrt(sqrt(a))); }

 float4 APrxPQToGamma2(float4 a) { return Quart(a); }
 float4 APrxPQToLinear(float4 a) { return Oct(a); }
 float4 APrxLoGamma2ToPQ(float4 a) { return asfloat(uint4((asuint(float4(a)) >> AU4_x(uint(2))) + AU4_x(uint(0x2F9A4E46)))); }
 float4 APrxMedGamma2ToPQ(float4 a) { float4 b = asfloat(uint4((asuint(float4(a)) >> AU4_x(uint(2))) + AU4_x(uint(0x2F9A4E46)))); float4 b4 = Quart(b); return b - b * (b4 - a) / (AF1_x(float(4.0)) * b4); }
 float4 APrxHighGamma2ToPQ(float4 a) { return sqrt(sqrt(a)); }
 float4 APrxLoLinearToPQ(float4 a) { return asfloat(uint4((asuint(float4(a)) >> AU4_x(uint(3))) + AU4_x(uint(0x378D8723)))); }
 float4 APrxMedLinearToPQ(float4 a) { float4 b = asfloat(uint4((asuint(float4(a)) >> AU4_x(uint(3))) + AU4_x(uint(0x378D8723)))); float4 b8 = Oct(b); return b - b * (b8 - a) / (AF1_x(float(8.0)) * b8); }
 float4 APrxHighLinearToPQ(float4 a) { return sqrt(sqrt(sqrt(a))); }
#line 1927 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_a.h"
  float APSinF1(float x){return x*abs(x)-x;}
  float2 APSinF2(float2 x){return x*abs(x)-x;}
  float APCosF1(float x){x=AFractF1(x*AF1_x(float(0.5))+AF1_x(float(0.75)));x=x*AF1_x(float(2.0))-AF1_x(float(1.0));return APSinF1(x);}
  float2 APCosF2(float2 x){x=AFractF2(x*AF2_x(float(0.5))+AF2_x(float(0.75)));x=x*AF2_x(float(2.0))-AF2_x(float(1.0));return APSinF2(x);}
  float2 APSinCosF1(float x){float y=AFractF1(x*AF1_x(float(0.5))+AF1_x(float(0.75)));y=y*AF1_x(float(2.0))-AF1_x(float(1.0));return APSinF2(float2(x,y));}
#line 1968 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_a.h"
  uint AZolAndU1(uint x,uint y){return min(x,y);}
  uint2 AZolAndU2(uint2 x,uint2 y){return min(x,y);}
  uint3 AZolAndU3(uint3 x,uint3 y){return min(x,y);}
  uint4 AZolAndU4(uint4 x,uint4 y){return min(x,y);}

  uint AZolNotU1(uint x){return x^AU1_x(uint(1));}
  uint2 AZolNotU2(uint2 x){return x^AU2_x(uint(1));}
  uint3 AZolNotU3(uint3 x){return x^AU3_x(uint(1));}
  uint4 AZolNotU4(uint4 x){return x^AU4_x(uint(1));}

  uint AZolOrU1(uint x,uint y){return max(x,y);}
  uint2 AZolOrU2(uint2 x,uint2 y){return max(x,y);}
  uint3 AZolOrU3(uint3 x,uint3 y){return max(x,y);}
  uint4 AZolOrU4(uint4 x,uint4 y){return max(x,y);}

  uint AZolF1ToU1(float x){return uint(x);}
  uint2 AZolF2ToU2(float2 x){return uint2(x);}
  uint3 AZolF3ToU3(float3 x){return uint3(x);}
  uint4 AZolF4ToU4(float4 x){return uint4(x);}


  uint AZolNotF1ToU1(float x){return uint(AF1_x(float(1.0))-x);}
  uint2 AZolNotF2ToU2(float2 x){return uint2(AF2_x(float(1.0))-x);}
  uint3 AZolNotF3ToU3(float3 x){return uint3(AF3_x(float(1.0))-x);}
  uint4 AZolNotF4ToU4(float4 x){return uint4(AF4_x(float(1.0))-x);}

  float AZolU1ToF1(uint x){return float(x);}
  float2 AZolU2ToF2(uint2 x){return float2(x);}
  float3 AZolU3ToF3(uint3 x){return float3(x);}
  float4 AZolU4ToF4(uint4 x){return float4(x);}

  float AZolAndF1(float x,float y){return min(x,y);}
  float2 AZolAndF2(float2 x,float2 y){return min(x,y);}
  float3 AZolAndF3(float3 x,float3 y){return min(x,y);}
  float4 AZolAndF4(float4 x,float4 y){return min(x,y);}

  float ASolAndNotF1(float x,float y){return (-x)*y+AF1_x(float(1.0));}
  float2 ASolAndNotF2(float2 x,float2 y){return (-x)*y+AF2_x(float(1.0));}
  float3 ASolAndNotF3(float3 x,float3 y){return (-x)*y+AF3_x(float(1.0));}
  float4 ASolAndNotF4(float4 x,float4 y){return (-x)*y+AF4_x(float(1.0));}

  float AZolAndOrF1(float x,float y,float z){return ASatF1(x*y+z);}
  float2 AZolAndOrF2(float2 x,float2 y,float2 z){return ASatF2(x*y+z);}
  float3 AZolAndOrF3(float3 x,float3 y,float3 z){return ASatF3(x*y+z);}
  float4 AZolAndOrF4(float4 x,float4 y,float4 z){return ASatF4(x*y+z);}

  float AZolGtZeroF1(float x){return ASatF1(x*AF1_x(float(asfloat(uint(0x7f800000u)))));}
  float2 AZolGtZeroF2(float2 x){return ASatF2(x*AF2_x(float(asfloat(uint(0x7f800000u)))));}
  float3 AZolGtZeroF3(float3 x){return ASatF3(x*AF3_x(float(asfloat(uint(0x7f800000u)))));}
  float4 AZolGtZeroF4(float4 x){return ASatF4(x*AF4_x(float(asfloat(uint(0x7f800000u)))));}

  float AZolNotF1(float x){return AF1_x(float(1.0))-x;}
  float2 AZolNotF2(float2 x){return AF2_x(float(1.0))-x;}
  float3 AZolNotF3(float3 x){return AF3_x(float(1.0))-x;}
  float4 AZolNotF4(float4 x){return AF4_x(float(1.0))-x;}

  float AZolOrF1(float x,float y){return max(x,y);}
  float2 AZolOrF2(float2 x,float2 y){return max(x,y);}
  float3 AZolOrF3(float3 x,float3 y){return max(x,y);}
  float4 AZolOrF4(float4 x,float4 y){return max(x,y);}

  float AZolSelF1(float x,float y,float z){float r=(-x)*z+z;return x*y+r;}
  float2 AZolSelF2(float2 x,float2 y,float2 z){float2 r=(-x)*z+z;return x*y+r;}
  float3 AZolSelF3(float3 x,float3 y,float3 z){float3 r=(-x)*z+z;return x*y+r;}
  float4 AZolSelF4(float4 x,float4 y,float4 z){float4 r=(-x)*z+z;return x*y+r;}

  float AZolSignedF1(float x){return ASatF1(x*AF1_x(float(asfloat(uint(0xff800000u)))));}
  float2 AZolSignedF2(float2 x){return ASatF2(x*AF2_x(float(asfloat(uint(0xff800000u)))));}
  float3 AZolSignedF3(float3 x){return ASatF3(x*AF3_x(float(asfloat(uint(0xff800000u)))));}
  float4 AZolSignedF4(float4 x){return ASatF4(x*AF4_x(float(asfloat(uint(0xff800000u)))));}

  float AZolZeroPassF1(float x,float y){return asfloat(uint((asuint(float(x))!=AU1_x(uint(0)))?AU1_x(uint(0)):asuint(float(y))));}
float2 AZolZeroPassF2(float2 x,float2 y){return asfloat(uint2(select(asuint(float2(x))!=AU2_x(uint(0)),AU2_x(uint(0)),asuint(float2(y)))));}
float3 AZolZeroPassF3(float3 x,float3 y){return asfloat(uint3(select(asuint(float3(x))!=AU3_x(uint(0)),AU3_x(uint(0)),asuint(float3(y)))));}
float4 AZolZeroPassF4(float4 x,float4 y){return asfloat(uint4(select(asuint(float4(x))!=AU4_x(uint(0)),AU4_x(uint(0)),asuint(float4(y)))));}
#line 2166 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_a.h"
  float ATo709F1(float c){float3 j=float3(0.018*4.5,4.5,0.45);float2 k=float2(1.099,-0.099);
   return clamp(j.x ,c*j.y ,pow(c,j.z )*k.x +k.y );}
  float2 ATo709F2(float2 c){float3 j=float3(0.018*4.5,4.5,0.45);float2 k=float2(1.099,-0.099);
   return clamp(j.xx ,c*j.yy ,pow(c,j.zz )*k.xx +k.yy );}
  float3 ATo709F3(float3 c){float3 j=float3(0.018*4.5,4.5,0.45);float2 k=float2(1.099,-0.099);
   return clamp(j.xxx,c*j.yyy,pow(c,j.zzz)*k.xxx+k.yyy);}


  float AToGammaF1(float c,float rcpX){return pow(c,AF1_x(float(rcpX)));}
  float2 AToGammaF2(float2 c,float rcpX){return pow(c,AF2_x(float(rcpX)));}
  float3 AToGammaF3(float3 c,float rcpX){return pow(c,AF3_x(float(rcpX)));}

  float AToPqF1(float x){float p=pow(x,AF1_x(float(0.159302)));
   return pow((AF1_x(float(0.835938))+AF1_x(float(18.8516))*p)/(AF1_x(float(1.0))+AF1_x(float(18.6875))*p),AF1_x(float(78.8438)));}
  float2 AToPqF1(float2 x){float2 p=pow(x,AF2_x(float(0.159302)));
   return pow((AF2_x(float(0.835938))+AF2_x(float(18.8516))*p)/(AF2_x(float(1.0))+AF2_x(float(18.6875))*p),AF2_x(float(78.8438)));}
  float3 AToPqF1(float3 x){float3 p=pow(x,AF3_x(float(0.159302)));
   return pow((AF3_x(float(0.835938))+AF3_x(float(18.8516))*p)/(AF3_x(float(1.0))+AF3_x(float(18.6875))*p),AF3_x(float(78.8438)));}

  float AToSrgbF1(float c){float3 j=float3(0.0031308*12.92,12.92,1.0/2.4);float2 k=float2(1.055,-0.055);
   return clamp(j.x ,c*j.y ,pow(c,j.z )*k.x +k.y );}
  float2 AToSrgbF2(float2 c){float3 j=float3(0.0031308*12.92,12.92,1.0/2.4);float2 k=float2(1.055,-0.055);
   return clamp(j.xx ,c*j.yy ,pow(c,j.zz )*k.xx +k.yy );}
  float3 AToSrgbF3(float3 c){float3 j=float3(0.0031308*12.92,12.92,1.0/2.4);float2 k=float2(1.055,-0.055);
   return clamp(j.xxx,c*j.yyy,pow(c,j.zzz)*k.xxx+k.yyy);}

  float AToTwoF1(float c){return sqrt(c);}
  float2 AToTwoF2(float2 c){return sqrt(c);}
  float3 AToTwoF3(float3 c){return sqrt(c);}

  float AToThreeF1(float c){return pow(c,AF1_x(float(1.0/3.0)));}
  float2 AToThreeF2(float2 c){return pow(c,AF2_x(float(1.0/3.0)));}
  float3 AToThreeF3(float3 c){return pow(c,AF3_x(float(1.0/3.0)));}




  float AFrom709F1(float c){float3 j=float3(0.081/4.5,1.0/4.5,1.0/0.45);float2 k=float2(1.0/1.099,0.099/1.099);
   return AZolSelF1(AZolSignedF1(c-j.x ),c*j.y ,pow(c*k.x +k.y ,j.z ));}
  float2 AFrom709F2(float2 c){float3 j=float3(0.081/4.5,1.0/4.5,1.0/0.45);float2 k=float2(1.0/1.099,0.099/1.099);
   return AZolSelF2(AZolSignedF2(c-j.xx ),c*j.yy ,pow(c*k.xx +k.yy ,j.zz ));}
  float3 AFrom709F3(float3 c){float3 j=float3(0.081/4.5,1.0/4.5,1.0/0.45);float2 k=float2(1.0/1.099,0.099/1.099);
   return AZolSelF3(AZolSignedF3(c-j.xxx),c*j.yyy,pow(c*k.xxx+k.yyy,j.zzz));}

  float AFromGammaF1(float c,float x){return pow(c,AF1_x(float(x)));}
  float2 AFromGammaF2(float2 c,float x){return pow(c,AF2_x(float(x)));}
  float3 AFromGammaF3(float3 c,float x){return pow(c,AF3_x(float(x)));}

  float AFromPqF1(float x){float p=pow(x,AF1_x(float(0.0126833)));
   return pow(ASatF1(p-AF1_x(float(0.835938)))/(AF1_x(float(18.8516))-AF1_x(float(18.6875))*p),AF1_x(float(6.27739)));}
  float2 AFromPqF1(float2 x){float2 p=pow(x,AF2_x(float(0.0126833)));
   return pow(ASatF2(p-AF2_x(float(0.835938)))/(AF2_x(float(18.8516))-AF2_x(float(18.6875))*p),AF2_x(float(6.27739)));}
  float3 AFromPqF1(float3 x){float3 p=pow(x,AF3_x(float(0.0126833)));
   return pow(ASatF3(p-AF3_x(float(0.835938)))/(AF3_x(float(18.8516))-AF3_x(float(18.6875))*p),AF3_x(float(6.27739)));}


  float AFromSrgbF1(float c){float3 j=float3(0.04045/12.92,1.0/12.92,2.4);float2 k=float2(1.0/1.055,0.055/1.055);
   return AZolSelF1(AZolSignedF1(c-j.x ),c*j.y ,pow(c*k.x +k.y ,j.z ));}
  float2 AFromSrgbF2(float2 c){float3 j=float3(0.04045/12.92,1.0/12.92,2.4);float2 k=float2(1.0/1.055,0.055/1.055);
   return AZolSelF2(AZolSignedF2(c-j.xx ),c*j.yy ,pow(c*k.xx +k.yy ,j.zz ));}
  float3 AFromSrgbF3(float3 c){float3 j=float3(0.04045/12.92,1.0/12.92,2.4);float2 k=float2(1.0/1.055,0.055/1.055);
   return AZolSelF3(AZolSignedF3(c-j.xxx),c*j.yyy,pow(c*k.xxx+k.yyy,j.zzz));}

  float AFromTwoF1(float c){return c*c;}
  float2 AFromTwoF2(float2 c){return c*c;}
  float3 AFromTwoF3(float3 c){return c*c;}

  float AFromThreeF1(float c){return c*c*c;}
  float2 AFromThreeF2(float2 c){return c*c*c;}
  float3 AFromThreeF3(float3 c){return c*c*c;}
#line 2304 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_a.h"
 uint2 ARmp8x8(uint a){return uint2(ABfe(a,1u,3u),ABfiM(ABfe(a,3u,3u),a,1u));}
#line 2322 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_a.h"
 uint2 ARmpRed8x8(uint a){return uint2(ABfiM(ABfe(a,2u,3u),a,1u),ABfiM(ABfe(a,3u,3u),ABfe(a,1u,2u),2u));}
#line 2609 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_a.h"
 float2 opAAbsF2(out float2 d,in float2 a){d=abs(a);return d;}
 float3 opAAbsF3(out float3 d,in float3 a){d=abs(a);return d;}
 float4 opAAbsF4(out float4 d,in float4 a){d=abs(a);return d;}

 float2 opAAddF2(out float2 d,in float2 a,in float2 b){d=a+b;return d;}
 float3 opAAddF3(out float3 d,in float3 a,in float3 b){d=a+b;return d;}
 float4 opAAddF4(out float4 d,in float4 a,in float4 b){d=a+b;return d;}

 float2 opAAddOneF2(out float2 d,in float2 a,float b){d=a+AF2_x(float(b));return d;}
 float3 opAAddOneF3(out float3 d,in float3 a,float b){d=a+AF3_x(float(b));return d;}
 float4 opAAddOneF4(out float4 d,in float4 a,float b){d=a+AF4_x(float(b));return d;}

 float2 opACpyF2(out float2 d,in float2 a){d=a;return d;}
 float3 opACpyF3(out float3 d,in float3 a){d=a;return d;}
 float4 opACpyF4(out float4 d,in float4 a){d=a;return d;}

 float2 opALerpF2(out float2 d,in float2 a,in float2 b,in float2 c){d=ALerpF2(a,b,c);return d;}
 float3 opALerpF3(out float3 d,in float3 a,in float3 b,in float3 c){d=ALerpF3(a,b,c);return d;}
 float4 opALerpF4(out float4 d,in float4 a,in float4 b,in float4 c){d=ALerpF4(a,b,c);return d;}

 float2 opALerpOneF2(out float2 d,in float2 a,in float2 b,float c){d=ALerpF2(a,b,AF2_x(float(c)));return d;}
 float3 opALerpOneF3(out float3 d,in float3 a,in float3 b,float c){d=ALerpF3(a,b,AF3_x(float(c)));return d;}
 float4 opALerpOneF4(out float4 d,in float4 a,in float4 b,float c){d=ALerpF4(a,b,AF4_x(float(c)));return d;}

 float2 opAMaxF2(out float2 d,in float2 a,in float2 b){d=max(a,b);return d;}
 float3 opAMaxF3(out float3 d,in float3 a,in float3 b){d=max(a,b);return d;}
 float4 opAMaxF4(out float4 d,in float4 a,in float4 b){d=max(a,b);return d;}

 float2 opAMinF2(out float2 d,in float2 a,in float2 b){d=min(a,b);return d;}
 float3 opAMinF3(out float3 d,in float3 a,in float3 b){d=min(a,b);return d;}
 float4 opAMinF4(out float4 d,in float4 a,in float4 b){d=min(a,b);return d;}

 float2 opAMulF2(out float2 d,in float2 a,in float2 b){d=a*b;return d;}
 float3 opAMulF3(out float3 d,in float3 a,in float3 b){d=a*b;return d;}
 float4 opAMulF4(out float4 d,in float4 a,in float4 b){d=a*b;return d;}

 float2 opAMulOneF2(out float2 d,in float2 a,float b){d=a*AF2_x(float(b));return d;}
 float3 opAMulOneF3(out float3 d,in float3 a,float b){d=a*AF3_x(float(b));return d;}
 float4 opAMulOneF4(out float4 d,in float4 a,float b){d=a*AF4_x(float(b));return d;}

 float2 opANegF2(out float2 d,in float2 a){d=-a;return d;}
 float3 opANegF3(out float3 d,in float3 a){d=-a;return d;}
 float4 opANegF4(out float4 d,in float4 a){d=-a;return d;}

 float2 opARcpF2(out float2 d,in float2 a){d=ARcpF2(a);return d;}
 float3 opARcpF3(out float3 d,in float3 a){d=ARcpF3(a);return d;}
 float4 opARcpF4(out float4 d,in float4 a){d=ARcpF4(a);return d;}
#line 17 "C:\\github\\Spectrum\\workdir\\shaders\\FSR.hlsl"
#line 65 "C:\\github\\Spectrum\\workdir\\shaders\\FSR.hlsl"
float4 FsrEasuRF(float2 p) { float4 res = InputTexture.GatherRed(linearClampSampler, p, int2(0, 0)); return res; }
float4 FsrEasuGF(float2 p) { float4 res = InputTexture.GatherGreen(linearClampSampler, p, int2(0, 0)); return res; }
float4 FsrEasuBF(float2 p) { float4 res = InputTexture.GatherBlue(linearClampSampler, p, int2(0, 0)); return res; }



#line 1 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_fsr1.h"
#line 156 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_fsr1.h"
         void FsrEasuCon(
out uint4 con0,
out uint4 con1,
out uint4 con2,
out uint4 con3,

float inputViewportInPixelsX,
float inputViewportInPixelsY,

float inputSizeInPixelsX,
float inputSizeInPixelsY,

float outputSizeInPixelsX,
float outputSizeInPixelsY){

 con0[0]=asuint(float(inputViewportInPixelsX*ARcpF1(outputSizeInPixelsX)));
 con0[1]=asuint(float(inputViewportInPixelsY*ARcpF1(outputSizeInPixelsY)));
 con0[2]=asuint(float(AF1_x(float(0.5))*inputViewportInPixelsX*ARcpF1(outputSizeInPixelsX)-AF1_x(float(0.5))));
 con0[3]=asuint(float(AF1_x(float(0.5))*inputViewportInPixelsY*ARcpF1(outputSizeInPixelsY)-AF1_x(float(0.5))));


 con1[0]=asuint(float(ARcpF1(inputSizeInPixelsX)));
 con1[1]=asuint(float(ARcpF1(inputSizeInPixelsY)));
#line 193 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_fsr1.h"
 con1[2]=asuint(float(AF1_x(float(1.0))*ARcpF1(inputSizeInPixelsX)));
 con1[3]=asuint(float(AF1_x(float(-1.0))*ARcpF1(inputSizeInPixelsY)));

 con2[0]=asuint(float(AF1_x(float(-1.0))*ARcpF1(inputSizeInPixelsX)));
 con2[1]=asuint(float(AF1_x(float(2.0))*ARcpF1(inputSizeInPixelsY)));
 con2[2]=asuint(float(AF1_x(float(1.0))*ARcpF1(inputSizeInPixelsX)));
 con2[3]=asuint(float(AF1_x(float(2.0))*ARcpF1(inputSizeInPixelsY)));
 con3[0]=asuint(float(AF1_x(float(0.0))*ARcpF1(inputSizeInPixelsX)));
 con3[1]=asuint(float(AF1_x(float(4.0))*ARcpF1(inputSizeInPixelsY)));
 con3[2]=con3[3]=0;}


         void FsrEasuConOffset(
    out uint4 con0,
    out uint4 con1,
    out uint4 con2,
    out uint4 con3,

    float inputViewportInPixelsX,
    float inputViewportInPixelsY,

    float inputSizeInPixelsX,
    float inputSizeInPixelsY,

    float outputSizeInPixelsX,
    float outputSizeInPixelsY,

    float inputOffsetInPixelsX,
    float inputOffsetInPixelsY) {
    FsrEasuCon(con0, con1, con2, con3, inputViewportInPixelsX, inputViewportInPixelsY, inputSizeInPixelsX, inputSizeInPixelsY, outputSizeInPixelsX, outputSizeInPixelsY);
    con0[2] = asuint(float(AF1_x(float(0.5)) * inputViewportInPixelsX * ARcpF1(outputSizeInPixelsX) - AF1_x(float(0.5)) + inputOffsetInPixelsX));
    con0[3] = asuint(float(AF1_x(float(0.5)) * inputViewportInPixelsY * ARcpF1(outputSizeInPixelsY) - AF1_x(float(0.5)) + inputOffsetInPixelsY));
}
#line 234 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_fsr1.h"
 float4 FsrEasuRF(float2 p);
 float4 FsrEasuGF(float2 p);
 float4 FsrEasuBF(float2 p);


 void FsrEasuTapF(
 inout float3 aC,
 inout float aW,
 float2 off,
 float2 dir,
 float2 len,
 float lob,
 float clp,
 float3 c){

  float2 v;
  v.x=(off.x*( dir.x))+(off.y*dir.y);
  v.y=(off.x*(-dir.y))+(off.y*dir.x);

  v*=len;

  float d2=v.x*v.x+v.y*v.y;

  d2=min(d2,clp);







  float wB=AF1_x(float(2.0/5.0))*d2+AF1_x(float(-1.0));
  float wA=lob*d2+AF1_x(float(-1.0));
  wB*=wB;
  wA*=wA;
  wB=AF1_x(float(25.0/16.0))*wB+AF1_x(float(-(25.0/16.0-1.0)));
  float w=wB*wA;

  aC+=c*w;aW+=w;}


 void FsrEasuSetF(
 inout float2 dir,
 inout float len,
 float2 pp,
 bool biS,bool biT,bool biU,bool biV,
 float lA,float lB,float lC,float lD,float lE){



  float w = AF1_x(float(0.0));
  if(biS)w=(AF1_x(float(1.0))-pp.x)*(AF1_x(float(1.0))-pp.y);
  if(biT)w= pp.x *(AF1_x(float(1.0))-pp.y);
  if(biU)w=(AF1_x(float(1.0))-pp.x)* pp.y ;
  if(biV)w= pp.x * pp.y ;






  float dc=lD-lC;
  float cb=lC-lB;
  float lenX=max(abs(dc),abs(cb));
  lenX=APrxLoRcpF1(lenX);
  float dirX=lD-lB;
  dir.x+=dirX*w;
  lenX=ASatF1(abs(dirX)*lenX);
  lenX*=lenX;
  len+=lenX*w;

  float ec=lE-lC;
  float ca=lC-lA;
  float lenY=max(abs(ec),abs(ca));
  lenY=APrxLoRcpF1(lenY);
  float dirY=lE-lA;
  dir.y+=dirY*w;
  lenY=ASatF1(abs(dirY)*lenY);
  lenY*=lenY;
  len+=lenY*w;}

 void FsrEasuF(
 out float3 pix,
 uint2 ip,
 uint4 con0,
 uint4 con1,
 uint4 con2,
 uint4 con3){


  float2 pp=float2(ip)*asfloat(uint2(con0.xy))+asfloat(uint2(con0.zw));
  float2 fp=floor(pp);
  pp-=fp;
#line 344 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_fsr1.h"
  float2 p0=fp*asfloat(uint2(con1.xy))+asfloat(uint2(con1.zw));

  float2 p1=p0+asfloat(uint2(con2.xy));
  float2 p2=p0+asfloat(uint2(con2.zw));
  float2 p3=p0+asfloat(uint2(con3.xy));
  float4 bczzR=FsrEasuRF(p0);
  float4 bczzG=FsrEasuGF(p0);
  float4 bczzB=FsrEasuBF(p0);
  float4 ijfeR=FsrEasuRF(p1);
  float4 ijfeG=FsrEasuGF(p1);
  float4 ijfeB=FsrEasuBF(p1);
  float4 klhgR=FsrEasuRF(p2);
  float4 klhgG=FsrEasuGF(p2);
  float4 klhgB=FsrEasuBF(p2);
  float4 zzonR=FsrEasuRF(p3);
  float4 zzonG=FsrEasuGF(p3);
  float4 zzonB=FsrEasuBF(p3);


  float4 bczzL=bczzB*AF4_x(float(0.5))+(bczzR*AF4_x(float(0.5))+bczzG);
  float4 ijfeL=ijfeB*AF4_x(float(0.5))+(ijfeR*AF4_x(float(0.5))+ijfeG);
  float4 klhgL=klhgB*AF4_x(float(0.5))+(klhgR*AF4_x(float(0.5))+klhgG);
  float4 zzonL=zzonB*AF4_x(float(0.5))+(zzonR*AF4_x(float(0.5))+zzonG);

  float bL=bczzL.x;
  float cL=bczzL.y;
  float iL=ijfeL.x;
  float jL=ijfeL.y;
  float fL=ijfeL.z;
  float eL=ijfeL.w;
  float kL=klhgL.x;
  float lL=klhgL.y;
  float hL=klhgL.z;
  float gL=klhgL.w;
  float oL=zzonL.z;
  float nL=zzonL.w;

  float2 dir=AF2_x(float(0.0));
  float len=AF1_x(float(0.0));
  FsrEasuSetF(dir,len,pp,true, false,false,false,bL,eL,fL,gL,jL);
  FsrEasuSetF(dir,len,pp,false,true ,false,false,cL,fL,gL,hL,kL);
  FsrEasuSetF(dir,len,pp,false,false,true ,false,fL,iL,jL,kL,nL);
  FsrEasuSetF(dir,len,pp,false,false,false,true ,gL,jL,kL,lL,oL);


  float2 dir2=dir*dir;
  float dirR=dir2.x+dir2.y;
  bool zro=dirR<AF1_x(float(1.0/32768.0));
  dirR=APrxLoRsqF1(dirR);
  dirR=zro?AF1_x(float(1.0)):dirR;
  dir.x=zro?AF1_x(float(1.0)):dir.x;
  dir*=AF2_x(float(dirR));

  len=len*AF1_x(float(0.5));
  len*=len;

  float stretch=(dir.x*dir.x+dir.y*dir.y)*APrxLoRcpF1(max(abs(dir.x),abs(dir.y)));



  float2 len2=float2(AF1_x(float(1.0))+(stretch-AF1_x(float(1.0)))*len,AF1_x(float(1.0))+AF1_x(float(-0.5))*len);


  float lob=AF1_x(float(0.5))+AF1_x(float((1.0/4.0-0.04)-0.5))*len;

  float clp=APrxLoRcpF1(lob);






  float3 min4=min(AMin3F3(float3(ijfeR.z,ijfeG.z,ijfeB.z),float3(klhgR.w,klhgG.w,klhgB.w),float3(ijfeR.y,ijfeG.y,ijfeB.y)),
               float3(klhgR.x,klhgG.x,klhgB.x));
  float3 max4=max(AMax3F3(float3(ijfeR.z,ijfeG.z,ijfeB.z),float3(klhgR.w,klhgG.w,klhgB.w),float3(ijfeR.y,ijfeG.y,ijfeB.y)),
               float3(klhgR.x,klhgG.x,klhgB.x));

  float3 aC=AF3_x(float(0.0));
  float aW=AF1_x(float(0.0));
  FsrEasuTapF(aC,aW,float2( 0.0,-1.0)-pp,dir,len2,lob,clp,float3(bczzR.x,bczzG.x,bczzB.x));
  FsrEasuTapF(aC,aW,float2( 1.0,-1.0)-pp,dir,len2,lob,clp,float3(bczzR.y,bczzG.y,bczzB.y));
  FsrEasuTapF(aC,aW,float2(-1.0, 1.0)-pp,dir,len2,lob,clp,float3(ijfeR.x,ijfeG.x,ijfeB.x));
  FsrEasuTapF(aC,aW,float2( 0.0, 1.0)-pp,dir,len2,lob,clp,float3(ijfeR.y,ijfeG.y,ijfeB.y));
  FsrEasuTapF(aC,aW,float2( 0.0, 0.0)-pp,dir,len2,lob,clp,float3(ijfeR.z,ijfeG.z,ijfeB.z));
  FsrEasuTapF(aC,aW,float2(-1.0, 0.0)-pp,dir,len2,lob,clp,float3(ijfeR.w,ijfeG.w,ijfeB.w));
  FsrEasuTapF(aC,aW,float2( 1.0, 1.0)-pp,dir,len2,lob,clp,float3(klhgR.x,klhgG.x,klhgB.x));
  FsrEasuTapF(aC,aW,float2( 2.0, 1.0)-pp,dir,len2,lob,clp,float3(klhgR.y,klhgG.y,klhgB.y));
  FsrEasuTapF(aC,aW,float2( 2.0, 0.0)-pp,dir,len2,lob,clp,float3(klhgR.z,klhgG.z,klhgB.z));
  FsrEasuTapF(aC,aW,float2( 1.0, 0.0)-pp,dir,len2,lob,clp,float3(klhgR.w,klhgG.w,klhgB.w));
  FsrEasuTapF(aC,aW,float2( 1.0, 2.0)-pp,dir,len2,lob,clp,float3(zzonR.z,zzonG.z,zzonB.z));
  FsrEasuTapF(aC,aW,float2( 0.0, 2.0)-pp,dir,len2,lob,clp,float3(zzonR.w,zzonG.w,zzonB.w));


  pix=min(max4,max(min4,aC*AF3_x(float(ARcpF1(aW)))));}
#line 662 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_fsr1.h"
         void FsrRcasCon(
out uint4 con,

float sharpness){

 sharpness=exp2(float(-sharpness));
 float2 hSharp=float2(sharpness,sharpness);
 con[0]=asuint(float(sharpness));
 con[1]=AU1_AH2_AF2_x(float2(hSharp));
 con[2]=0;
 con[3]=0;}
#line 1014 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_fsr1.h"
 void FsrLfgaF(inout float3 c,float3 t,float a){c+=(t*AF3_x(float(a)))*min(AF3_x(float(1.0))-c,c);}
#line 1043 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_fsr1.h"
 void FsrSrtmF(inout float3 c){c*=AF3_x(float(ARcpF1(AMax3F1(c.r,c.g,c.b)+AF1_x(float(1.0)))));}

 void FsrSrtmInvF(inout float3 c){c*=AF3_x(float(ARcpF1(max(AF1_x(float(1.0/32768.0)),AF1_x(float(1.0))-AMax3F1(c.r,c.g,c.b)))));}
#line 1086 "C:\\github\\Spectrum\\workdir\\shaders/fsr/ffx_fsr1.h"
 float FsrTepdDitF(uint2 p,uint f){
  float x=AF1_x(float(p.x+f));
  float y=AF1_x(float(p.y));

  float a=AF1_x(float((1.0+sqrt(5.0))/2.0));

  float b=AF1_x(float(1.0/3.69));
  x=x*a+(y*b);
  return AFractF1(x);}




 void FsrTepdC8F(inout float3 c,float dit){
  float3 n=sqrt(c);
  n=floor(n*AF3_x(float(255.0)))*AF3_x(float(1.0/255.0));
  float3 a=n*n;
  float3 b=n+AF3_x(float(1.0/255.0));b=b*b;



  float3 r=(c-b)*APrxMedRcpF3(a-b);


  c=ASatF3(n+AGtZeroF3(AF3_x(float(dit))-r)*AF3_x(float(1.0/255.0)));}




 void FsrTepdC10F(inout float3 c,float dit){
  float3 n=sqrt(c);
  n=floor(n*AF3_x(float(1023.0)))*AF3_x(float(1.0/1023.0));
  float3 a=n*n;
  float3 b=n+AF3_x(float(1.0/1023.0));b=b*b;
  float3 r=(c-b)*APrxMedRcpF3(a-b);
  c=ASatF3(n+AGtZeroF3(AF3_x(float(dit))-r)*AF3_x(float(1.0/1023.0)));}
#line 70 "C:\\github\\Spectrum\\workdir\\shaders\\FSR.hlsl"




void CurrFilter(int2 pos)
{





 float3 c;
 FsrEasuF(c, pos, Const0, Const1, Const2, Const3);
 OutputTexture[pos] = float4(c, 1);


}

[numthreads(64,1,1)]
void CS(uint3 LocalThreadId : SV_GroupThreadID, uint3 WorkGroupId : SV_GroupID, uint3 Dtid : SV_DispatchThreadID)
{

 uint2 gxy = ARmp8x8(LocalThreadId.x) + uint2(WorkGroupId.x << 4u, WorkGroupId.y << 4u);
 CurrFilter(gxy);
 gxy.x += 8u;
 CurrFilter(gxy);
 gxy.y += 8u;
 CurrFilter(gxy);
 gxy.x -= 8u;
 CurrFilter(gxy);

}
