// MaterialX standard library, "genspectrum" target.
//
// MaterialXLibrary (sources/RenderSystem/Materials/MaterialXLibrary.cpp)
// treats a nodedef as implemented when this file defines a function named
// mx_<category> -- the nodedef's node category (add, mix, smoothstep, ...).
// One name serves every type variant (float, vector2/3/4, color3/4):
// MaterialXNode coerces each argument to its nodedef-declared type before
// the call, so HLSL overload resolution picks the variant.
//
// The "FA" (float amount) nodedefs, e.g. ND_add_vector3FA, take a float as
// their second operand. DXC does not apply partial ordering between
// (T, T) and (T, float) templates -- the scalar case is ambiguous -- so
// those forms live under mx_<category>_fa instead.
//
// Nothing here may depend on common.hlsl: the material preview shaders
// include this file without it. Semantics follow MaterialX's own genglsl
// library (libraries/stdlib/genglsl). Where HLSL and GLSL differ: GLSL mod()
// is x - y*floor(x/y), not fmod(); mix() is lerp(); fract() is frac().

#ifndef MX_STDLIB_HLSL
#define MX_STDLIB_HLSL

// ---- math: binary arithmetic ------------------------------------------------

template<typename T> T mx_add(T a, T b) { return a + b; }
template<typename T> T mx_add_fa(T a, float b) { return a + b; }
template<typename T> T mx_subtract(T a, T b) { return a - b; }
template<typename T> T mx_subtract_fa(T a, float b) { return a - b; }
template<typename T> T mx_multiply(T a, T b) { return a * b; }
template<typename T> T mx_multiply_fa(T a, float b) { return a * b; }
template<typename T> T mx_divide(T a, T b) { return a / b; }
template<typename T> T mx_divide_fa(T a, float b) { return a / b; }

template<typename T> T mx_modulo(T a, T b) { return a - b * floor(a / b); }
template<typename T> T mx_modulo_fa(T a, float b) { return a - b * floor(a / b); }

template<typename T> T mx_power(T a, T b) { return pow(a, b); }
template<typename T> T mx_power_fa(T a, float b) { return pow(a, b); }

template<typename T> T mx_min(T a, T b) { return min(a, b); }
template<typename T> T mx_min_fa(T a, float b) { return min(a, b); }
template<typename T> T mx_max(T a, T b) { return max(a, b); }
template<typename T> T mx_max_fa(T a, float b) { return max(a, b); }

template<typename T> T mx_atan2(T iny, T inx) { return atan2(iny, inx); }
template<typename T> T mx_atan2_fa(T iny, float inx) { return atan2(iny, inx); }

// ---- math: unary ------------------------------------------------------------

template<typename T> T mx_absval(T a) { return abs(a); }
template<typename T> T mx_floor(T a) { return floor(a); }
template<typename T> T mx_ceil(T a) { return ceil(a); }
template<typename T> T mx_sin(T a) { return sin(a); }
template<typename T> T mx_cos(T a) { return cos(a); }
template<typename T> T mx_tan(T a) { return tan(a); }
template<typename T> T mx_asin(T a) { return asin(a); }
template<typename T> T mx_acos(T a) { return acos(a); }
template<typename T> T mx_sqrt(T a) { return sqrt(a); }
template<typename T> T mx_ln(T a) { return log(a); }
template<typename T> T mx_exp(T a) { return exp(a); }
template<typename T> T mx_sign(T a) { return sign(a); }
template<typename T> T mx_invert(T a, T amount) { return amount - a; }
template<typename T> T mx_invert_fa(T a, float amount) { return amount - a; }

// ---- math: ranges -----------------------------------------------------------

template<typename T> T mx_clamp(T a, T lo, T hi) { return clamp(a, lo, hi); }
template<typename T> T mx_clamp_fa(T a, float lo, float hi) { return clamp(a, lo, hi); }

// MaterialX orders smoothstep's inputs (in, low, high); HLSL's takes (low, high, x).
template<typename T> T mx_smoothstep(T a, T lo, T hi) { return smoothstep(lo, hi, a); }
template<typename T> T mx_smoothstep_fa(T a, float lo, float hi) { return smoothstep(lo, hi, a); }

// remap: map a from [inlow, inhigh] to [outlow, outhigh].
template<typename T> T mx_remap(T a, T inlow, T inhigh, T outlow, T outhigh)
{
    T t = (a - inlow) / (inhigh - inlow);
    return outlow + t * (outhigh - outlow);
}
template<typename T> T mx_remap_fa(T a, float inlow, float inhigh, float outlow, float outhigh)
{
    T t = (a - inlow) / (inhigh - inlow);
    return outlow + t * (outhigh - outlow);
}

// range: remap + optional gamma and clamp.
template<typename T> T mx_range(T a, T inlow, T inhigh, T gamma, T outlow, T outhigh, bool doclamp)
{
    T result = (a - inlow) / (inhigh - inlow);
    result = pow(max(result, 0), 1.0 / gamma);
    result = outlow + result * (outhigh - outlow);
    return doclamp ? clamp(result, outlow, outhigh) : result;
}

// ---- compositing ------------------------------------------------------------

// The nodedef declares its inputs as (fg, bg, mix) -- not the bg-first order
// MaterialX's documentation uses -- and arguments are passed positionally in
// nodedef order, so fg comes first here. The vector forms take either a
// per-channel amount (same type as fg/bg) or a single float; the non-template
// overloads handle the latter, since a (T, T, float) template is ambiguous
// with (T, T, T) when T is float.
template<typename T> T mx_mix(T fg, T bg, T amount) { return lerp(bg, fg, amount); }
float2 mx_mix(float2 fg, float2 bg, float amount) { return lerp(bg, fg, amount); }
float3 mx_mix(float3 fg, float3 bg, float amount) { return lerp(bg, fg, amount); }
float4 mx_mix(float4 fg, float4 bg, float amount) { return lerp(bg, fg, amount); }

// ---- logic and selection ----------------------------------------------------

template<typename T> T mx_ifgreater(float value1, float value2, T in1, T in2) { return value1 > value2 ? in1 : in2; }
template<typename T> T mx_ifgreatereq(float value1, float value2, T in1, T in2) { return value1 >= value2 ? in1 : in2; }
template<typename T> T mx_ifequal(float value1, float value2, T in1, T in2) { return value1 == value2 ? in1 : in2; }

// Boolean-output forms: no in1/in2, the node just returns the comparison.
bool mx_ifgreater(float value1, float value2) { return value1 > value2; }
bool mx_ifgreatereq(float value1, float value2) { return value1 >= value2; }
bool mx_ifequal(float value1, float value2) { return value1 == value2; }

// ---- vector math ------------------------------------------------------------

template<typename T> float mx_dotproduct(T a, T b) { return dot(a, b); }
float3 mx_crossproduct(float3 a, float3 b) { return cross(a, b); }
template<typename T> float mx_magnitude(T a) { return length(a); }
template<typename T> T mx_normalize(T a) { return normalize(a); }

// ---- channels / conversion --------------------------------------------------

float2 mx_combine2(float a, float b) { return float2(a, b); }
float4 mx_combine2(float3 a, float b) { return float4(a, b); }
float4 mx_combine2(float2 a, float2 b) { return float4(a, b); }
float3 mx_combine3(float a, float b, float c) { return float3(a, b, c); }
float4 mx_combine4(float a, float b, float c, float d) { return float4(a, b, c, d); }

// separate2/3/4 have one output per channel, so they use out-parameters.
void mx_separate2(float2 v, out float outx, out float outy) { outx = v.x; outy = v.y; }
void mx_separate3(float3 v, out float outx, out float outy, out float outz) { outx = v.x; outy = v.y; outz = v.z; }
void mx_separate4(float4 v, out float outr, out float outg, out float outb, out float outa) { outr = v.x; outg = v.y; outb = v.z; outa = v.w; }

float mx_luminance(float3 c, float3 lumacoeffs) { return dot(c, lumacoeffs); }
float mx_luminance(float4 c, float4 lumacoeffs) { return dot(c.xyz, lumacoeffs.xyz); }

// ---- procedural -------------------------------------------------------------

// ramplr/ramptb: linear ramp from valuel to valuer (valuet to valueb) over
// the texcoord's u (v), clamped to [0,1].
template<typename T> T mx_ramplr(T valuel, T valuer, float2 texcoord)
{
    return lerp(valuel, valuer, clamp(texcoord.x, 0.0, 1.0));
}
template<typename T> T mx_ramptb(T valuet, T valueb, float2 texcoord)
{
    return lerp(valuet, valueb, clamp(texcoord.y, 0.0, 1.0));
}

// checkerboard: color1 on even cells, color2 on odd, after scaling and
// offsetting the texcoord. Matches mx_checkerboard.glsl.
float3 mx_checkerboard(float3 color1, float3 color2, float2 uvscale, float2 uvoffset, float2 texcoord)
{
    float2 uv = texcoord * uvscale + uvoffset;
    uv = floor(uv);
    float check = fmod(uv.x + uv.y, 2.0);
    return lerp(color1, color2, abs(check));
}

#endif
