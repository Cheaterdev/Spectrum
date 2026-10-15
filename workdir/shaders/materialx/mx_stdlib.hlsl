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

// ---- geometry ---------------------------------------------------------------
//
// World-space shading frame of the point being shaded. COMPILED_FUNC only
// receives position and texcoord, so a stage that has a frame sets these
// before calling it (mx_set_geometry); stages without one (VSM depth) keep the
// defaults. MaterialXNode reads them for inputs whose nodedef default is
// Nworld / Tworld / Bworld, and for the standalone normal/tangent/bitangent nodes.

static float3 mx_geom_normal    = float3(0, 0, 1);
static float3 mx_geom_tangent   = float3(1, 0, 0);
static float3 mx_geom_bitangent = float3(0, 1, 0);

void mx_set_geometry(float3 normal, float3 tangent, float3 bitangent)
{
    mx_geom_normal    = normal;
    mx_geom_tangent   = tangent;
    mx_geom_bitangent = bitangent;
}

// The geometry nodes take MaterialX's `space` string, which the graph has no
// type for (see MaterialXLibrary), so they always return world space.
float3 mx_normal()              { return mx_geom_normal; }
float3 mx_tangent(int index)    { return mx_geom_tangent; }
float3 mx_bitangent(int index)  { return mx_geom_bitangent; }

// Tangent-space normal (encoded 0..1, as the texture stores it) to the frame
// N/T/B. All-zero input means "no map": the flat normal.
float3 mx_normalmap(float3 value, float2 normal_scale, float3 N, float3 T, float3 B)
{
    value = dot(value, value) == 0.0 ? float3(0, 0, 1) : value * 2.0 - 1.0;
    return normalize(T * value.x * normal_scale.x + B * value.y * normal_scale.y + N * value.z);
}

float3 mx_normalmap(float3 value, float normal_scale, float3 N, float3 T, float3 B)
{
    return mx_normalmap(value, float2(normal_scale, normal_scale), N, T, B);
}

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

// ---- noise ------------------------------------------------------------------
//
// Port of MaterialX's genglsl/lib/mx_noise.glsl, itself derived from Open
// Shading Language's oslnoise.h (Copyright (c) 2009-2010 Sony Pictures
// Imageworks Inc., et al.; BSD 3-Clause, notice reproduced in MaterialX's
// library). Results are meant to match OSL/MaterialX bit for bit, so the hash
// and the gradient tables must not be "improved". Helpers use the mxn_ prefix:
// anything named mx_<word>( would be mistaken for a node implementation.

int mxn_floor(float x) { return (int)floor(x); }

float mxn_floorfrac(float x, out int i)
{
    i = mxn_floor(x);
    return x - (float)i;
}

float mxn_select(bool b, float t, float f) { return b ? t : f; }
float mxn_negate_if(float val, bool b) { return b ? -val : val; }

float mxn_bilerp(float v0, float v1, float v2, float v3, float s, float t)
{
    float s1 = 1.0 - s;
    return (1.0 - t) * (v0 * s1 + v1 * s) + t * (v2 * s1 + v3 * s);
}
float3 mxn_bilerp(float3 v0, float3 v1, float3 v2, float3 v3, float s, float t)
{
    float s1 = 1.0 - s;
    return (1.0 - t) * (v0 * s1 + v1 * s) + t * (v2 * s1 + v3 * s);
}

float mxn_trilerp(float v0, float v1, float v2, float v3, float v4, float v5, float v6, float v7, float s, float t, float r)
{
    float s1 = 1.0 - s;
    float t1 = 1.0 - t;
    float r1 = 1.0 - r;
    return r1 * (t1 * (v0 * s1 + v1 * s) + t * (v2 * s1 + v3 * s)) +
           r  * (t1 * (v4 * s1 + v5 * s) + t * (v6 * s1 + v7 * s));
}
float3 mxn_trilerp(float3 v0, float3 v1, float3 v2, float3 v3, float3 v4, float3 v5, float3 v6, float3 v7, float s, float t, float r)
{
    float s1 = 1.0 - s;
    float t1 = 1.0 - t;
    float r1 = 1.0 - r;
    return r1 * (t1 * (v0 * s1 + v1 * s) + t * (v2 * s1 + v3 * s)) +
           r  * (t1 * (v4 * s1 + v5 * s) + t * (v6 * s1 + v7 * s));
}

float mxn_gradient_float(uint hash, float x, float y)
{
    uint h = hash & 7u;
    float u = mxn_select(h < 4u, x, y);
    float v = 2.0 * mxn_select(h < 4u, y, x);
    return mxn_negate_if(u, (h & 1u) != 0u) + mxn_negate_if(v, (h & 2u) != 0u);
}
float mxn_gradient_float(uint hash, float x, float y, float z)
{
    uint h = hash & 15u;
    float u = mxn_select(h < 8u, x, y);
    float v = mxn_select(h < 4u, y, mxn_select((h == 12u) || (h == 14u), x, z));
    return mxn_negate_if(u, (h & 1u) != 0u) + mxn_negate_if(v, (h & 2u) != 0u);
}
float3 mxn_gradient_vec3(uint3 hash, float x, float y)
{
    return float3(mxn_gradient_float(hash.x, x, y), mxn_gradient_float(hash.y, x, y), mxn_gradient_float(hash.z, x, y));
}
float3 mxn_gradient_vec3(uint3 hash, float x, float y, float z)
{
    return float3(mxn_gradient_float(hash.x, x, y, z), mxn_gradient_float(hash.y, x, y, z), mxn_gradient_float(hash.z, x, y, z));
}

// Normalise the unnormalised gradients above; measured, not chosen.
float  mxn_gradient_scale2d(float v)  { return 0.6616 * v; }
float  mxn_gradient_scale3d(float v)  { return 0.9820 * v; }
float3 mxn_gradient_scale2d(float3 v) { return 0.6616 * v; }
float3 mxn_gradient_scale3d(float3 v) { return 0.9820 * v; }

uint mxn_rotl32(uint x, int k) { return (x << k) | (x >> (32 - k)); }

void mxn_bjmix(inout uint a, inout uint b, inout uint c)
{
    a -= c; a ^= mxn_rotl32(c, 4);  c += b;
    b -= a; b ^= mxn_rotl32(a, 6);  a += c;
    c -= b; c ^= mxn_rotl32(b, 8);  b += a;
    a -= c; a ^= mxn_rotl32(c, 16); c += b;
    b -= a; b ^= mxn_rotl32(a, 19); a += c;
    c -= b; c ^= mxn_rotl32(b, 4);  b += a;
}

uint mxn_bjfinal(uint a, uint b, uint c)
{
    c ^= b; c -= mxn_rotl32(b, 14);
    a ^= c; a -= mxn_rotl32(c, 11);
    b ^= a; b -= mxn_rotl32(a, 25);
    c ^= b; c -= mxn_rotl32(b, 16);
    a ^= c; a -= mxn_rotl32(c, 4);
    b ^= a; b -= mxn_rotl32(a, 14);
    c ^= b; c -= mxn_rotl32(b, 24);
    return c;
}

float mxn_bits_to_01(uint bits) { return (float)bits / (float)0xffffffffu; }

float mxn_fade(float t) { return t * t * t * (t * (t * 6.0 - 15.0) + 10.0); }

uint mxn_hash_int(int x)
{
    uint seed = 0xdeadbeefu + (1u << 2u) + 13u;
    return mxn_bjfinal(seed + (uint)x, seed, seed);
}

uint mxn_hash_int(int x, int y)
{
    uint a = 0xdeadbeefu + (2u << 2u) + 13u;
    uint b = a;
    uint c = a;
    a += (uint)x;
    b += (uint)y;
    return mxn_bjfinal(a, b, c);
}

uint mxn_hash_int(int x, int y, int z)
{
    uint a = 0xdeadbeefu + (3u << 2u) + 13u;
    uint b = a;
    uint c = a;
    a += (uint)x;
    b += (uint)y;
    c += (uint)z;
    return mxn_bjfinal(a, b, c);
}

uint3 mxn_hash_vec3(int x, int y)
{
    uint h = mxn_hash_int(x, y);
    return uint3(h & 0xFFu, (h >> 8) & 0xFFu, (h >> 16) & 0xFFu);
}

uint3 mxn_hash_vec3(int x, int y, int z)
{
    uint h = mxn_hash_int(x, y, z);
    return uint3(h & 0xFFu, (h >> 8) & 0xFFu, (h >> 16) & 0xFFu);
}

float mxn_perlin_float(float2 p)
{
    int X, Y;
    float fx = mxn_floorfrac(p.x, X);
    float fy = mxn_floorfrac(p.y, Y);
    float u = mxn_fade(fx);
    float v = mxn_fade(fy);
    float result = mxn_bilerp(
        mxn_gradient_float(mxn_hash_int(X,     Y    ), fx,       fy      ),
        mxn_gradient_float(mxn_hash_int(X + 1, Y    ), fx - 1.0, fy      ),
        mxn_gradient_float(mxn_hash_int(X,     Y + 1), fx,       fy - 1.0),
        mxn_gradient_float(mxn_hash_int(X + 1, Y + 1), fx - 1.0, fy - 1.0),
        u, v);
    return mxn_gradient_scale2d(result);
}

float mxn_perlin_float(float3 p)
{
    int X, Y, Z;
    float fx = mxn_floorfrac(p.x, X);
    float fy = mxn_floorfrac(p.y, Y);
    float fz = mxn_floorfrac(p.z, Z);
    float u = mxn_fade(fx);
    float v = mxn_fade(fy);
    float w = mxn_fade(fz);
    float result = mxn_trilerp(
        mxn_gradient_float(mxn_hash_int(X,     Y,     Z    ), fx,       fy,       fz      ),
        mxn_gradient_float(mxn_hash_int(X + 1, Y,     Z    ), fx - 1.0, fy,       fz      ),
        mxn_gradient_float(mxn_hash_int(X,     Y + 1, Z    ), fx,       fy - 1.0, fz      ),
        mxn_gradient_float(mxn_hash_int(X + 1, Y + 1, Z    ), fx - 1.0, fy - 1.0, fz      ),
        mxn_gradient_float(mxn_hash_int(X,     Y,     Z + 1), fx,       fy,       fz - 1.0),
        mxn_gradient_float(mxn_hash_int(X + 1, Y,     Z + 1), fx - 1.0, fy,       fz - 1.0),
        mxn_gradient_float(mxn_hash_int(X,     Y + 1, Z + 1), fx,       fy - 1.0, fz - 1.0),
        mxn_gradient_float(mxn_hash_int(X + 1, Y + 1, Z + 1), fx - 1.0, fy - 1.0, fz - 1.0),
        u, v, w);
    return mxn_gradient_scale3d(result);
}

float3 mxn_perlin_vec3(float2 p)
{
    int X, Y;
    float fx = mxn_floorfrac(p.x, X);
    float fy = mxn_floorfrac(p.y, Y);
    float u = mxn_fade(fx);
    float v = mxn_fade(fy);
    float3 result = mxn_bilerp(
        mxn_gradient_vec3(mxn_hash_vec3(X,     Y    ), fx,       fy      ),
        mxn_gradient_vec3(mxn_hash_vec3(X + 1, Y    ), fx - 1.0, fy      ),
        mxn_gradient_vec3(mxn_hash_vec3(X,     Y + 1), fx,       fy - 1.0),
        mxn_gradient_vec3(mxn_hash_vec3(X + 1, Y + 1), fx - 1.0, fy - 1.0),
        u, v);
    return mxn_gradient_scale2d(result);
}

float3 mxn_perlin_vec3(float3 p)
{
    int X, Y, Z;
    float fx = mxn_floorfrac(p.x, X);
    float fy = mxn_floorfrac(p.y, Y);
    float fz = mxn_floorfrac(p.z, Z);
    float u = mxn_fade(fx);
    float v = mxn_fade(fy);
    float w = mxn_fade(fz);
    float3 result = mxn_trilerp(
        mxn_gradient_vec3(mxn_hash_vec3(X,     Y,     Z    ), fx,       fy,       fz      ),
        mxn_gradient_vec3(mxn_hash_vec3(X + 1, Y,     Z    ), fx - 1.0, fy,       fz      ),
        mxn_gradient_vec3(mxn_hash_vec3(X,     Y + 1, Z    ), fx,       fy - 1.0, fz      ),
        mxn_gradient_vec3(mxn_hash_vec3(X + 1, Y + 1, Z    ), fx - 1.0, fy - 1.0, fz      ),
        mxn_gradient_vec3(mxn_hash_vec3(X,     Y,     Z + 1), fx,       fy,       fz - 1.0),
        mxn_gradient_vec3(mxn_hash_vec3(X + 1, Y,     Z + 1), fx - 1.0, fy,       fz - 1.0),
        mxn_gradient_vec3(mxn_hash_vec3(X,     Y + 1, Z + 1), fx,       fy - 1.0, fz - 1.0),
        mxn_gradient_vec3(mxn_hash_vec3(X + 1, Y + 1, Z + 1), fx - 1.0, fy - 1.0, fz - 1.0),
        u, v, w);
    return mxn_gradient_scale3d(result);
}

float mxn_cell_float(float2 p)
{
    return mxn_bits_to_01(mxn_hash_int(mxn_floor(p.x), mxn_floor(p.y)));
}

float mxn_cell_float(float3 p)
{
    return mxn_bits_to_01(mxn_hash_int(mxn_floor(p.x), mxn_floor(p.y), mxn_floor(p.z)));
}

float3 mxn_cell_vec3(float2 p)
{
    int ix = mxn_floor(p.x);
    int iy = mxn_floor(p.y);
    return float3(
        mxn_bits_to_01(mxn_hash_int(ix, iy, 0)),
        mxn_bits_to_01(mxn_hash_int(ix, iy, 1)),
        mxn_bits_to_01(mxn_hash_int(ix, iy, 2)));
}

float3 mxn_cell_vec3(float3 p)
{
    uint a = 0xdeadbeefu + (4u << 2u) + 13u;
    uint b = a;
    uint c = a;
    a += (uint)mxn_floor(p.x);
    b += (uint)mxn_floor(p.y);
    c += (uint)mxn_floor(p.z);
    mxn_bjmix(a, b, c);
    return float3(
        mxn_bits_to_01(mxn_bjfinal(a,      b, c)),
        mxn_bits_to_01(mxn_bjfinal(a + 1u, b, c)),
        mxn_bits_to_01(mxn_bjfinal(a + 2u, b, c)));
}

float mxn_fractal_float(float2 p, int octaves, float lacunarity, float diminish)
{
    float result = 0.0;
    float amplitude = 1.0;
    for (int i = 0; i < octaves; ++i)
    {
        result += amplitude * mxn_perlin_float(p);
        amplitude *= diminish;
        p *= lacunarity;
    }
    return result;
}

float mxn_fractal_float(float3 p, int octaves, float lacunarity, float diminish)
{
    float result = 0.0;
    float amplitude = 1.0;
    for (int i = 0; i < octaves; ++i)
    {
        result += amplitude * mxn_perlin_float(p);
        amplitude *= diminish;
        p *= lacunarity;
    }
    return result;
}

float3 mxn_fractal_vec3(float2 p, int octaves, float lacunarity, float diminish)
{
    float3 result = 0.0;
    float amplitude = 1.0;
    for (int i = 0; i < octaves; ++i)
    {
        result += amplitude * mxn_perlin_vec3(p);
        amplitude *= diminish;
        p *= lacunarity;
    }
    return result;
}

float3 mxn_fractal_vec3(float3 p, int octaves, float lacunarity, float diminish)
{
    float3 result = 0.0;
    float amplitude = 1.0;
    for (int i = 0; i < octaves; ++i)
    {
        result += amplitude * mxn_perlin_vec3(p);
        amplitude *= diminish;
        p *= lacunarity;
    }
    return result;
}

float2 mxn_worley_cell_position(int x, int y, int xoff, int yoff, float jitter)
{
    float2 off = mxn_cell_vec3(float2(x + xoff, y + yoff)).xy;
    off -= 0.5;
    off *= jitter;
    off += 0.5;
    return float2(x, y) + off;
}

float3 mxn_worley_cell_position(int x, int y, int z, int xoff, int yoff, int zoff, float jitter)
{
    float3 off = mxn_cell_vec3(float3(x + xoff, y + yoff, z + zoff));
    off -= 0.5;
    off *= jitter;
    off += 0.5;
    return float3(x, y, z) + off;
}

// metric: 0 Euclidean (squared here, rooted by the caller), 1 distance^2,
// 2 Manhattan, 3 Chebyshev. The stdlib nodedefs only ever pass 0.
float mxn_worley_distance(float2 p, int x, int y, int xoff, int yoff, float jitter, int metric)
{
    float2 diff = mxn_worley_cell_position(x, y, xoff, yoff, jitter) - p;
    if (metric == 2)
        return abs(diff.x) + abs(diff.y);
    if (metric == 3)
        return max(abs(diff.x), abs(diff.y));
    return dot(diff, diff);
}

float mxn_worley_distance(float3 p, int x, int y, int z, int xoff, int yoff, int zoff, float jitter, int metric)
{
    float3 diff = mxn_worley_cell_position(x, y, z, xoff, yoff, zoff, jitter) - p;
    if (metric == 2)
        return abs(diff.x) + abs(diff.y) + abs(diff.z);
    if (metric == 3)
        return max(max(abs(diff.x), abs(diff.y)), abs(diff.z));
    return dot(diff, diff);
}

// Nearest three feature distances of a 2D cell neighbourhood, plus the
// offset of the nearest feature point (for the "Solid" style).
void mxn_worley_search(float2 p, float jitter, int metric, out float3 sqdist, out float2 minpos, out float2 localpos)
{
    int X, Y;
    localpos = float2(mxn_floorfrac(p.x, X), mxn_floorfrac(p.y, Y));
    sqdist = 1e6;
    minpos = 0;

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float dist = mxn_worley_distance(localpos, x, y, X, Y, jitter, metric);
            float2 cellpos = mxn_worley_cell_position(x, y, X, Y, jitter) - localpos;

            if (dist < sqdist.x)
            {
                sqdist.z = sqdist.y;
                sqdist.y = sqdist.x;
                sqdist.x = dist;
                minpos = cellpos;
            }
            else if (dist < sqdist.y)
            {
                sqdist.z = sqdist.y;
                sqdist.y = dist;
            }
            else if (dist < sqdist.z)
            {
                sqdist.z = dist;
            }
        }
    }
}

void mxn_worley_search(float3 p, float jitter, int metric, out float3 sqdist, out float3 minpos, out float3 localpos)
{
    int X, Y, Z;
    localpos = float3(mxn_floorfrac(p.x, X), mxn_floorfrac(p.y, Y), mxn_floorfrac(p.z, Z));
    sqdist = 1e6;
    minpos = 0;

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            for (int z = -1; z <= 1; ++z)
            {
                float dist = mxn_worley_distance(localpos, x, y, z, X, Y, Z, jitter, metric);
                float3 cellpos = mxn_worley_cell_position(x, y, z, X, Y, Z, jitter) - localpos;

                if (dist < sqdist.x)
                {
                    sqdist.z = sqdist.y;
                    sqdist.y = sqdist.x;
                    sqdist.x = dist;
                    minpos = cellpos;
                }
                else if (dist < sqdist.y)
                {
                    sqdist.z = sqdist.y;
                    sqdist.y = dist;
                }
                else if (dist < sqdist.z)
                {
                    sqdist.z = dist;
                }
            }
        }
    }
}

// One search serves the float, vector2 and vector3 forms: they differ only in
// how many of the three nearest distances they return.
float3 mxn_worley(float2 p, float jitter, int style)
{
    float3 sqdist; float2 minpos; float2 localpos;
    mxn_worley_search(p, jitter, 0, sqdist, minpos, localpos);

    if (style == 1)
        return mxn_cell_vec3(minpos + p);

    return sqrt(sqdist);
}

float3 mxn_worley(float3 p, float jitter, int style)
{
    float3 sqdist; float3 minpos; float3 localpos;
    mxn_worley_search(p, jitter, 0, sqdist, minpos, localpos);

    if (style == 1)
        return mxn_cell_vec3(minpos + p);

    return sqrt(sqdist);
}

// Solid style of the scalar form hashes the cell with the scalar cell noise,
// which is not the first channel of the vec3 one; keep it separate.
float mxn_worley_float(float2 p, float jitter, int style)
{
    if (style == 1)
    {
        float3 sqdist; float2 minpos; float2 localpos;
        mxn_worley_search(p, jitter, 0, sqdist, minpos, localpos);
        return mxn_cell_float(minpos + p);
    }

    return mxn_worley(p, jitter, style).x;
}

float mxn_worley_float(float3 p, float jitter, int style)
{
    if (style == 1)
    {
        float3 sqdist; float3 minpos; float3 localpos;
        mxn_worley_search(p, jitter, 0, sqdist, minpos, localpos);
        return mxn_cell_float(minpos + p);
    }

    return mxn_worley(p, jitter, style).x;
}

// 4D cell noise, used by flake only.
uint mxn_hash_int(int x, int y, int z, int xx)
{
    uint a = 0xdeadbeefu + (4u << 2u) + 13u;
    uint b = a;
    uint c = a;
    a += (uint)x;
    b += (uint)y;
    c += (uint)z;
    mxn_bjmix(a, b, c);
    a += (uint)xx;
    return mxn_bjfinal(a, b, c);
}

float mxn_cell_float(float4 p)
{
    return mxn_bits_to_01(mxn_hash_int(mxn_floor(p.x), mxn_floor(p.y), mxn_floor(p.z), mxn_floor(p.w)));
}

float3 mxn_cell_vec3(float4 p)
{
    uint a = 0xdeadbeefu + (5u << 2u) + 13u;
    uint b = a;
    uint c = a;
    a += (uint)mxn_floor(p.x);
    b += (uint)mxn_floor(p.y);
    c += (uint)mxn_floor(p.z);
    mxn_bjmix(a, b, c);
    a += (uint)mxn_floor(p.w);
    return float3(
        mxn_bits_to_01(mxn_bjfinal(a, b,      c)),
        mxn_bits_to_01(mxn_bjfinal(a, b + 1u, c)),
        mxn_bits_to_01(mxn_bjfinal(a, b + 2u, c)));
}

// -- nodes --

float  mx_noise2d(float  amplitude, float pivot, float2 texcoord) { return mxn_perlin_float(texcoord) * amplitude + pivot; }
float2 mx_noise2d(float2 amplitude, float pivot, float2 texcoord) { return mxn_perlin_vec3(texcoord).xy * amplitude + pivot; }
float3 mx_noise2d(float3 amplitude, float pivot, float2 texcoord) { return mxn_perlin_vec3(texcoord) * amplitude + pivot; }
float4 mx_noise2d(float4 amplitude, float pivot, float2 texcoord)
{
    return float4(mxn_perlin_vec3(texcoord), mxn_perlin_float(texcoord + float2(19, 73))) * amplitude + pivot;
}

float  mx_noise3d(float  amplitude, float pivot, float3 position) { return mxn_perlin_float(position) * amplitude + pivot; }
float2 mx_noise3d(float2 amplitude, float pivot, float3 position) { return mxn_perlin_vec3(position).xy * amplitude + pivot; }
float3 mx_noise3d(float3 amplitude, float pivot, float3 position) { return mxn_perlin_vec3(position) * amplitude + pivot; }
float4 mx_noise3d(float4 amplitude, float pivot, float3 position)
{
    return float4(mxn_perlin_vec3(position), mxn_perlin_float(position + float3(19, 73, 29))) * amplitude + pivot;
}

float  mx_fractal2d(float  amplitude, int octaves, float lacunarity, float diminish, float2 texcoord)
{
    return mxn_fractal_float(texcoord, octaves, lacunarity, diminish) * amplitude;
}
float2 mx_fractal2d(float2 amplitude, int octaves, float lacunarity, float diminish, float2 texcoord)
{
    return float2(mxn_fractal_float(texcoord, octaves, lacunarity, diminish),
                  mxn_fractal_float(texcoord + float2(19, 193), octaves, lacunarity, diminish)) * amplitude;
}
float3 mx_fractal2d(float3 amplitude, int octaves, float lacunarity, float diminish, float2 texcoord)
{
    return mxn_fractal_vec3(texcoord, octaves, lacunarity, diminish) * amplitude;
}
float4 mx_fractal2d(float4 amplitude, int octaves, float lacunarity, float diminish, float2 texcoord)
{
    return float4(mxn_fractal_vec3(texcoord, octaves, lacunarity, diminish),
                  mxn_fractal_float(texcoord + float2(19, 193), octaves, lacunarity, diminish)) * amplitude;
}

float  mx_fractal3d(float  amplitude, int octaves, float lacunarity, float diminish, float3 position)
{
    return mxn_fractal_float(position, octaves, lacunarity, diminish) * amplitude;
}
float2 mx_fractal3d(float2 amplitude, int octaves, float lacunarity, float diminish, float3 position)
{
    return float2(mxn_fractal_float(position, octaves, lacunarity, diminish),
                  mxn_fractal_float(position + float3(19, 193, 17), octaves, lacunarity, diminish)) * amplitude;
}
float3 mx_fractal3d(float3 amplitude, int octaves, float lacunarity, float diminish, float3 position)
{
    return mxn_fractal_vec3(position, octaves, lacunarity, diminish) * amplitude;
}
float4 mx_fractal3d(float4 amplitude, int octaves, float lacunarity, float diminish, float3 position)
{
    return float4(mxn_fractal_vec3(position, octaves, lacunarity, diminish),
                  mxn_fractal_float(position + float3(19, 193, 17), octaves, lacunarity, diminish)) * amplitude;
}

float mx_cellnoise2d(float2 texcoord) { return mxn_cell_float(texcoord); }
float mx_cellnoise3d(float3 position) { return mxn_cell_float(position); }

// The worley nodedefs differ only in output type, so they are looked up as
// mx_<category>_<type> (see MaterialXLibrary::register_nodes).
float  mx_worleynoise2d_float(float2 texcoord, float jitter, int style)   { return mxn_worley_float(texcoord, jitter, style); }
float2 mx_worleynoise2d_vector2(float2 texcoord, float jitter, int style) { return mxn_worley(texcoord, jitter, style).xy; }
float3 mx_worleynoise2d_vector3(float2 texcoord, float jitter, int style) { return mxn_worley(texcoord, jitter, style); }
float  mx_worleynoise3d_float(float3 position, float jitter, int style)   { return mxn_worley_float(position, jitter, style); }
float2 mx_worleynoise3d_vector2(float3 position, float jitter, int style) { return mxn_worley(position, jitter, style).xy; }
float3 mx_worleynoise3d_vector3(float3 position, float jitter, int style) { return mxn_worley(position, jitter, style); }

// ---- color ------------------------------------------------------------------
// From MaterialX's lib/mx_hsv.glsl (also derived from OSL; Foley & van Dam).

float3 mxn_hsvtorgb(float3 hsv)
{
    float h = hsv.x;
    float s = hsv.y;
    float v = hsv.z;

    if (s < 0.0001)
        return float3(v, v, v);

    h = 6.0 * (h - floor(h));
    int hi = (int)trunc(h);
    float f = h - (float)hi;
    float p = v * (1.0 - s);
    float q = v * (1.0 - s * f);
    float t = v * (1.0 - s * (1.0 - f));

    if (hi == 0) return float3(v, t, p);
    if (hi == 1) return float3(q, v, p);
    if (hi == 2) return float3(p, v, t);
    if (hi == 3) return float3(p, q, v);
    if (hi == 4) return float3(t, p, v);
    return float3(v, p, q);
}

float3 mxn_rgbtohsv(float3 c)
{
    float r = c.x;
    float g = c.y;
    float b = c.z;
    float maxcomp = max(r, max(g, b));
    float delta = maxcomp - min(r, min(g, b));
    float s = maxcomp > 0.0 ? delta / maxcomp : 0.0;
    float h = 0.0;

    if (s > 0.0)
    {
        if (r >= maxcomp)      h = (g - b) / delta;
        else if (g >= maxcomp) h = 2.0 + (b - r) / delta;
        else                   h = 4.0 + (r - g) / delta;

        h *= 1.0 / 6.0;

        if (h < 0.0)
            h += 1.0;
    }

    return float3(h, s, maxcomp);
}

float3 mx_hsvtorgb(float3 c) { return mxn_hsvtorgb(c); }
float4 mx_hsvtorgb(float4 c) { return float4(mxn_hsvtorgb(c.xyz), c.w); }
float3 mx_rgbtohsv(float3 c) { return mxn_rgbtohsv(c); }
float4 mx_rgbtohsv(float4 c) { return float4(mxn_rgbtohsv(c.xyz), c.w); }

float4 mx_premult(float4 c)   { return float4(c.xyz * c.w, c.w); }
float4 mx_unpremult(float4 c) { return float4(c.xyz / c.w, c.w); }

// burn/dodge/disjointover: the "mix" blend of MaterialX compositing.
template<typename T> T mx_burn(T fg, T bg, float mixval)
{
    T r = mixval * (1.0 - (1.0 - bg) / fg) + (1.0 - mixval) * bg;
    return select(abs(fg) < 1e-8, (T)0, r);
}

template<typename T> T mx_dodge(T fg, T bg, float mixval)
{
    T r = mixval * (bg / (1.0 - fg)) + (1.0 - mixval) * bg;
    return select(abs(1.0 - fg) < 1e-8, (T)0, r);
}

float4 mx_disjointover(float4 fg, float4 bg, float mixval)
{
    float summed = fg.w + bg.w;
    float3 rgb;

    if (summed <= 1.0)
        rgb = fg.xyz + bg.xyz;
    else if (abs(bg.w) < 1e-8)
        rgb = 0.0;
    else
        rgb = fg.xyz + bg.xyz * ((1.0 - fg.w) / bg.w);

    float4 result = float4(rgb, min(summed, 1.0));
    return result * mixval + (1.0 - mixval) * bg;
}

// ---- geometry-independent transforms ----------------------------------------

float2 mx_rotate2d(float2 v, float amount)
{
    float a = radians(amount);
    float sa = sin(a);
    float ca = cos(a);
    return float2(ca * v.x + sa * v.y, -sa * v.x + ca * v.y);
}

// Rodrigues; MaterialX uses v' = v * M, hence the cross product order.
float3 mx_rotate3d(float3 v, float amount, float3 axis)
{
    axis = normalize(axis);
    float a = radians(amount);
    float s = sin(a);
    float c = cos(a);
    return v * c + cross(v, axis) * s + axis * dot(axis, v) * (1.0 - c);
}

// MaterialX antialiases the split edge with screen derivatives (mx_aastep).
// COMPILED_FUNC also runs in ray-tracing hit shaders, where ddx/ddy do not
// exist, so the edge here is a hard step.
template<typename T> T mx_splitlr(T valuel, T valuer, float center, float2 texcoord)
{
    return lerp(valuel, valuer, step(center, texcoord.x));
}

template<typename T> T mx_splittb(T valuet, T valueb, float center, float2 texcoord)
{
    return lerp(valueb, valuet, step(center, texcoord.y));
}

// ---- flake ------------------------------------------------------------------
// From lib/mx_flake.glsl. normal/tangent/bitangent are the shading frame; they
// default to geometry inputs that MaterialXNode does not provide yet, so the
// flake normal is only meaningful once those are linked.

float3 mxn_rotate_flake(float3 p, float3 i)
{
    float theta = 3.14159265358979 * 2.0 * i.x;
    float phi = 3.14159265358979 * 2.0 * i.y;
    float z = i.z * 2.0;
    float r = sqrt(z);
    float vx = sin(phi) * r;
    float vy = cos(phi) * r;
    float vz = sqrt(2.0 - z);
    float s_theta = sin(theta);
    float c_theta = cos(theta);
    float sx = vx * c_theta - vy * s_theta;
    float sy = vx * s_theta + vy * c_theta;

    // GLSL mat3(...) is column-major: the arguments below are the columns.
    return float3(vx * sx - c_theta, vx * sy - s_theta, vx * vz) * p.x +
           float3(vy * sx + s_theta, vy * sy - c_theta, vy * vz) * p.y +
           float3(vz * sx,           vz * sy,           1.0 - z) * p.z;
}

// Numerically fitted curve of flake density to per-cell probability.
float mxn_flake_density_to_probability(float x)
{
    const float4 abcd = float4(-26.19771808, 26.39663835, 85.53857017, -102.35069432);
    const float2 ef = float2(-101.42634862, 118.45082288);
    float xx = x * x;
    return (abcd.x * xx + abcd.y * x) / (abcd.z * xx * x + abcd.w * xx + ef.x * x + ef.y);
}

void mxn_flake(float size, float roughness, float coverage, float3 position,
               float3 normal, float3 tangent, float3 bitangent,
               out int id, out float rand, out float presence, out float3 flakenormal)
{
    float probability = mxn_flake_density_to_probability(clamp(coverage, 0.0, 1.0));
    const float flake_diameter = 1.5 / sqrt(3.0);

    float3 P = position / size;
    float3 base_P = floor(P);
    float flake_priority = 0.0;
    float3 flake_cell = 0.0;

    for (int i = -1; i < 2; ++i)
    {
        for (int j = -1; j < 2; ++j)
        {
            for (int k = -1; k < 2; ++k)
            {
                float3 cell_pos = base_P + float3(i, j, k);
                float3 PP = P - cell_pos - 0.5;

                if (dot(PP, PP) >= flake_diameter * flake_diameter * 3.0)
                    continue;

                if (mxn_cell_float(cell_pos) > probability)
                    continue;

                float priority = mxn_cell_float(float4(cell_pos, 3.0));

                if (priority < flake_priority)
                    continue;

                PP = mxn_rotate_flake(PP, mxn_cell_vec3(cell_pos));

                if (abs(PP.x) <= flake_diameter && abs(PP.y) <= flake_diameter && abs(PP.z) <= flake_diameter)
                {
                    flake_priority = priority;
                    flake_cell = cell_pos;
                }
            }
        }
    }

    id = 0;
    rand = 0.0;
    presence = 0.0;
    flakenormal = normal;

    if (flake_priority <= 0.0)
        return;

    float3 flake_noise = mxn_cell_vec3(float4(flake_cell, 2.0));
    float xi0 = flake_noise.x;
    float xi1 = flake_noise.y;
    rand = flake_noise.z;
    id = (int)(rand * 16777215.0);   // 2^24-1 is exact in float32
    presence = flake_priority;

    float phi = 3.14159265358979 * 2.0 * xi0;
    float tan_theta = roughness * roughness * sqrt(xi1) / sqrt(1.0 - xi1);
    float sin_theta = tan_theta / sqrt(1.0 + tan_theta * tan_theta);
    float cos_theta = sqrt(1.0 - sin_theta * sin_theta);
    flakenormal = normalize(tangent * cos(phi) * sin_theta + bitangent * sin(phi) * sin_theta + normal * cos_theta);
}

void mx_flake2d(float size, float roughness, float coverage, float2 texcoord,
                float3 normal, float3 tangent, float3 bitangent,
                out int id, out float rand, out float presence, out float3 flakenormal)
{
    mxn_flake(size, roughness, coverage, float3(texcoord, 0.0), normal, tangent, bitangent, id, rand, presence, flakenormal);
}

void mx_flake3d(float size, float roughness, float coverage, float3 position,
                float3 normal, float3 tangent, float3 bitangent,
                out int id, out float rand, out float presence, out float3 flakenormal)
{
    mxn_flake(size, roughness, coverage, position, normal, tangent, bitangent, id, rand, presence, flakenormal);
}

// Everything MaterialX defines as an inline expression or a nodegraph is
// generated (workdir/materialx/gen_generated_hlsl.py) and sees the functions above.
#include "mx_generated.hlsl"

#endif
