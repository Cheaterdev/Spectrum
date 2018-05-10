#define SIZE 16
#define FIX 8

struct camera_info
{
    matrix view;
    matrix proj;
    matrix view_proj;
    matrix inv_view;
    matrix inv_proj;
    matrix inv_view_proj;
    float3 position;
    float3 direction;
};


cbuffer cbCamera : register(b0)
{
    camera_info camera;
};


cbuffer cbSkyParams : register(b1)
{
	float3 light_dir;
};


float3 depth_to_wpos(float d, float2 tc, matrix mat)
{
    float4 P = mul(mat, float4(tc * float2(2, -2) + float2(-1, 1), d, 1));
    return P.xyz / P.w;
}

float3 depth_to_wpos_center(float d, float2 tc, matrix mat)
{
    float4 P = mul(mat, float4(tc, d, 1));
    return P.xyz / P.w;
}


Texture2D tex_transmittance: register(t0); //ground reflectance texture
Texture2D tex_irradiance: register(t1); //precomputed skylight irradiance (E table)
Texture3D tex_inscatter: register(t2); //precomputed inscattered light (S table)
Texture2D<float> depth_buffer : register(t3);

//extureCube cube_test : register(t7); //precomputed inscattered light (S table)
//Texture2D<float4> back : register(t7);



//RWTexture2D<float4> result: register(u0);



SamplerState LinearSampler: register(s0);
SamplerState PixelSampler : register(s1);

#include "sky_common.hlsl"

static const float ISun = 20.0;

static const float Scaler = 0.00001;
//inscattered light along ray x+tv, when sun in direction s (=S[L]-T(x,x0)S[L]|x0)
float4 inscatter(inout float3 x, inout float t, float3 v, float3 s, out float r, out float mu, out float3 attenuation)
{
    float4 result;
    r = length(x);
    mu = dot(x, v) / r;
    float d = -r * mu - sqrt(r * r * (mu * mu - 1.0) + Rt * Rt);

    if (d > 0.0)   // if x in space and ray intersects atmosphere
    {
        // move x to nearest intersection of ray with top atmosphere boundary
        x += d * v;
        t -= d;
        mu = (r * mu + d) / Rt;
        r = Rt;
        //return 1;
    }

    if (r <= Rt)
    {
        // if ray intersects atmosphere
        float nu = dot(v, s);
        float muS = dot(x, s) / r;
        float phaseR = phaseFunctionR(nu);
        float phaseM = phaseFunctionM(nu);
        float4 inscatter = max(texture4D(tex_inscatter, r, mu, muS, nu), 0.0);

        if (t > 0.0)
        {
            float3 x0 = x + t * v;
            float r0 = length(x0);
            float rMu0 = dot(x0, v);
            float mu0 = rMu0 / r0;
            float muS0 = dot(x0, s) / r0;
#ifdef FIX
            // avoids imprecision problems in transmittance computations based on textures
            attenuation = analyticTransmittance(r, mu, t);
#else
            attenuation = transmittance(r, mu, v, x0);
#endif

//return (r0-Rg)/10;
            if (r0 > Rg)
            {
                //  return inscatter;
                // computes S[L]-T(x,x0)S[L]|x0
                inscatter = max(inscatter - attenuation.rgbr * texture4D(tex_inscatter, r0, mu0, muS0, nu), 0.0);
#ifdef FIX
                // avoids imprecision problems near horizon by interpolating between two points above and below horizon
                const float EPS = 1;
                float muHoriz = -sqrt(1.0 - (Rg / r) * (Rg / r));

                if (abs(mu - muHoriz) < EPS)
                {
                    float a = ((mu - muHoriz) + EPS) / (2.0 * EPS);
                    mu = muHoriz - EPS;
                    r0 = sqrt(r * r + t * t + 2.0 * r * t * mu);
                    mu0 = (r * mu + t) / r0;
                    float4 inScatter0 = texture4D(tex_inscatter, r, mu, muS, nu);
                    float4 inScatter1 = texture4D(tex_inscatter, r0, mu0, muS0, nu);
                    float4 inScatterA = max(inScatter0 - attenuation.rgbr * inScatter1, 0.0);
                    mu = muHoriz + EPS;
                    r0 = sqrt(r * r + t * t + 2.0 * r * t * mu);
                    mu0 = (r * mu + t) / r0;
                    inScatter0 = texture4D(tex_inscatter, r, mu, muS, nu);
                    inScatter1 = texture4D(tex_inscatter, r0, mu0, muS0, nu);
                    float4 inScatterB = max(inScatter0 - attenuation.rgbr * inScatter1, 0.0);
                    inscatter = lerp(inScatterA, inScatterB, a);
                }

#endif
            }
        }

#ifdef FIX
        // avoids imprecision problems in Mie scattering when sun is below horizon
        //  inscatter.w *= smoothstep(0.00, 0.02, muS);
#endif
        result = float4(max(inscatter.rgb * phaseR + getMie(inscatter) * phaseM, 0.0) * ISun, 1);
        //result=r/1000000;//max(texture4D(tex_inscatter, r, mu, muS, nu), 0.0);
    }

    else     // x in space and ray looking in space
    {
        return 0;//  result = 0;//float3(0.0);
    }

    return result; //* ISun;
}

//ground radiance at end of ray x+tv, when sun in direction s
//attenuated bewteen ground and viewer (=R[L0]+R[L*])
float3 groundColor(float3 x, float t, float3 v, float3 s, float r, float mu, float3 attenuation)
{
    float3 result;

    if (t > 0.0)   // if ray hits ground surface
    {
        // ground reflectance at end of ray, x0
        float3 x0 = x + t * v;
        float r0 = length(x0);
        float3 n = x0 / r0;
        float2 coords = float2(atan2(n.y, n.x), acos(n.z)) * float2(0.5, 1.0) / M_PI + float2(0.5, 0.0);
        float4 reflectance = tex_transmittance.Sample(LinearSampler, coords) * float4(0.2, 0.2, 0.2, 1.0);

        if (r0 > Rg + 0.01)
            reflectance = float4(0.4, 0.4, 0.4, 0.0);

        // direct sun light (radiance) reaching x0
        float muS = dot(n, s);
        float3 sunLight = analyticTransmittance(r0, mu, t);
        // precomputed sky light (irradiance) (=E[L*]) at x0
        float3 groundSkyLight = irradiance(tex_irradiance, r0, muS);
        // light reflected at x0 (=(R[L0]+R[L*])/T(x,x0))
        float3 groundColor = reflectance.rgb * (max(muS, 0.0) * sunLight + groundSkyLight) * ISun / M_PI;

        // water specular color due to sunLight
        if (reflectance.w > 0.0)
        {
            float3 h = normalize(s - v);
            float fresnel = 0.02 + 0.98 * pow(1.0 - dot(-v, h), 5.0);
            float waterBrdf = fresnel * pow(max(dot(h, n), 0.0), 150.0);
            groundColor += reflectance.w * max(waterBrdf, 0.0) * sunLight * ISun;
        }

        result = attenuation * groundColor; //=R[L0]+R[L*]
    }

    else     // ray looking at the sky
    {
        result = 0;//float3(0.0);
    }

    return result;
}

// direct sun light for ray x+tv, when sun in direction s (=L0)
float3 sunColor(float3 x, float t, float3 v, float3 s, float r, float mu)
{
    if (t > 0.0)
    {
        return 0;//float3(0.0);
    }

    else
    {
        float3 transmittance = r <= Rt ? transmittanceWithShadow(r, mu) : float3(1.0, 1, 1); // T(x,xo)
        float isun = step(cos(M_PI / 180.0), dot(v, s)) * ISun; // Lsun
        return transmittance * isun; // Eq (9)
    }
}


float3 get_sky(float3 p, float3 v, float t)
{
    p.y = max(110, p.y);
    float3 x = Scaler * p + float3(0, Rg, 0);
    float r = 0;
    float mu = 0;
    float3 attenuation = 0;
    float3 s = light_dir;
    float3 inscatterColor = inscatter(x, t, v, s, r, mu, attenuation);
    float3 sunColor_ = sunColor(x, t, v, s, r, mu);
    //float3 ground = groundColor(x, t, v, s, r, mu, attenuation);
    return sunColor_ + inscatterColor;
}

#ifdef BUILD_FUNC_CS


[numthreads(SIZE, SIZE, 1)]
void CS(uint3 group_id :  SV_GroupID, uint3 thread_id : SV_GroupThreadID)
{
    const uint2 tc = (group_id.xy * SIZE + thread_id.xy);
    float2 dims;
    uint dummy;
    gbuffer[0].GetDimensions(dims.x, dims.y);

    if (dims.x < tc.x) return;

    if (dims.y < tc.y) return;

    float raw_z = depth_buffer[tc.xy];
    float3 p = depth_to_wpos(raw_z, float2(tc.xy) / dims, camera.inv_view_proj);
    float t = (raw_z < 1) * Scaler * length(p - camera.position);
    float3 v = normalize(p - camera.position);
    result[tc.xy] = float4(get_sky(p, v, t), 1);
}
#endif

struct quad_output
{
float4 pos : SV_POSITION;
float2 tc : TEXCOORD0;
float3 ray : TEXCOORD1;
};

quad_output VS(uint index : SV_VERTEXID)
{
    static float2 Pos[] =
    {
        float2(-1, 1),
        float2(1, 1),
        float2(-1, -1),
        float2(1, -1)
    };
    static float2 Tex[] =
    {

        float2(0, 0),
        float2(1, 0),
        float2(0, 1),
        float2(1, 1),
    };
    quad_output Output;
    Output.pos = float4(Pos[index], 0.3, 1);
    Output.tc = Tex[index];
    float4 r = mul(camera.inv_proj, float4(Pos[index], 1, 1));
    Output.ray = mul(camera.inv_view, r.xyz);
    //mul(camera.inv_view, float4(mul(camera.inv_view, float4(Pos[index], 1, 1)).xyz, 1.0));
    return Output;
}


float4 PS(quad_output i): SV_Target0
{ 
    float3 v = normalize(i.ray);
	float raw_z = depth_buffer.SampleLevel(PixelSampler, i.tc, 0).x;
    float3 p = depth_to_wpos(raw_z, i.tc.xy, camera.inv_view_proj);
    float t = (raw_z < 1) * Scaler * length(p - camera.position);



    /*  float3 normal = normalize(gbuffer[1].SampleLevel(PixelSampler, i.tc, 0).xyz*2-1);
      float3 diffuse = gbuffer[0].SampleLevel(PixelSampler, i.tc, 0).xyz;
      float4 spec_gloss = gbuffer[2].SampleLevel(PixelSampler, i.tc, 0)*2;
      float3 refl = reflect(p - camera.position, normal);
      float3 cube_color = diffuse/8+spec_gloss.xyz * (raw_z < 1) * cube_test.SampleLevel(LinearSampler, refl, saturate(1 - spec_gloss.w) * 8).xyz;
    */
    // float4 orig = back.SampleLevel(PixelSampler, i.tc, 0);
    return float4(/*orig.xyz+*/get_sky(camera.position, v, t), 1);
}