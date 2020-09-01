
struct quad_output
{
	float4 pos : SV_POSITION;
	float3 tc: TEXCOORD0;
};

#ifdef BUILD_FUNC_VS

#include "autogen/SkyFace.h"
static float2 Pos[] =
	{	
		float2(-1, -1),
		float2(-1, 1),
		float2(1, -1),
		float2(1, 1)
	};
	


static float3x3 mats[]=
{
float3x3(0,0,1,   0,1,0,   -1,0,0),//X+
float3x3(0,0,-1,  0,1,0,   1,0,0),//X-

float3x3(1,0,0,   0,0,1,   0,-1,0),//Y+
float3x3(1,0,0,   0,0,-1,  0,1,0),//Y-

float3x3(1,0,0,   0,1,0,   0,0,1), //Z+
float3x3(-1,0,0,   0,1,0,   0,0,-1)//Z-
};

quad_output VS(uint index : SV_VERTEXID)
{
	quad_output Output;
	Output.pos = float4(Pos[index], 0.99999, 1); 
	Output.tc = normalize(mul(mats[GetSkyFace().GetFace()],float3(Pos[index],1)));

	return Output;
}
#endif

#ifdef BUILD_FUNC_PS


#include "autogen/SkyData.h"
#include "autogen/FrameInfo.h"

static const Camera camera = GetFrameInfo().GetCamera();


#include "sky_common.hlsl"

static const float ISun = 20.0;

static const float Scaler = 0.0001;
//inscattered light along ray x+tv, when sun in direction s (=S[L]-T(x,x0)S[L]|x0)
float3 inscatter(inout float3 x, inout float t, float3 v, float3 s, out float r, out float mu, out float3 attenuation) {
    float3 result;
    r = length(x);
    mu = dot(x, v) / r;
    float d = -r * mu - sqrt(r * r * (mu * mu - 1.0) + Rt * Rt);
    if (d > 0.0) { // if x in space and ray intersects atmosphere
        // move x to nearest intersection of ray with top atmosphere boundary
        x += d * v;
        t -= d;
        mu = (r * mu + d) / Rt;
        r = Rt;
		//return 1;
    }
    if (r <= Rt) 
	{ // if ray intersects atmosphere
        float nu = dot(v, s);
        float muS = dot(x, s) / r;
        float phaseR = phaseFunctionR(nu);
        float phaseM = phaseFunctionM(nu);
        float4 inscatter = max(texture4D(tex_inscatter, r, mu, muS, nu), 0.0);

        if (t > 0.0) {
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

                // computes S[L]-T(x,x0)S[L]|x0
                inscatter = max(inscatter - attenuation.rgbr * texture4D(tex_inscatter, r0, mu0, muS0, nu), 0.0);
#ifdef FIX
                // avoids imprecision problems near horizon by interpolating between two points above and below horizon
                const float EPS = 0.04;
                float muHoriz = -sqrt(1.0 - (Rg / r) * (Rg / r));
                if (abs(mu - muHoriz) < EPS) {
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
        inscatter.w *= smoothstep(0.00, 0.02, muS);
#endif
        result = max(inscatter.rgb * phaseR + getMie(inscatter) * phaseM, 0.0);
		//result=r/1000000;//max(texture4D(tex_inscatter, r, mu, muS, nu), 0.0);
    } else { // x in space and ray looking in space
      return 0;//  result = 0;//float3(0.0);r
    }
    return result * ISun;
}

//ground radiance at end of ray x+tv, when sun in direction s
//attenuated bewteen ground and viewer (=R[L0]+R[L*])
float3 groundColor(float3 x, float t, float3 v, float3 s, float r, float mu, float3 attenuation)
{
    float3 result;
    if (t > 0.0) { // if ray hits ground surface
        // ground reflectance at end of ray, x0
        float3 x0 = x + t * v;
        float r0 = length(x0);
        float3 n = x0 / r0;
        float2 coords = float2(atan2(n.y, n.x), acos(n.z)) * float2(0.5, 1.0) / M_PI + float2(0.5, 0.0);
        float4 reflectance = tex_transmittance.Sample(LinearSampler,coords) * float4(0.2, 0.2, 0.2, 1.0);
        if (r0 > Rg + 0.01) {
            reflectance = float4(0.4, 0.4, 0.4, 0.0);
        }

        // direct sun light (radiance) reaching x0
        float muS = dot(n, s);
        float3 sunLight = 0;//transmittanceWithShadow(r0, muS);
			
        // precomputed sky light (irradiance) (=E[L*]) at x0
        float3 groundSkyLight = irradiance(tex_irradiance, r0, muS);
	
        // light reflected at x0 (=(R[L0]+R[L*])/T(x,x0))
        float3 groundColor = reflectance.rgb * (max(muS, 0.0) * sunLight + groundSkyLight) * ISun / M_PI;
			
        // water specular color due to sunLight
        if (reflectance.w > 0.0) {
            float3 h = normalize(s - v);
            float fresnel = 0.02 + 0.98 * pow(1.0 - dot(-v, h), 5.0);
            float waterBrdf = fresnel * pow(max(dot(h, n), 0.0), 150.0);
            groundColor += reflectance.w * max(waterBrdf, 0.0) * sunLight * ISun;
        }

        result = attenuation * groundColor; //=R[L0]+R[L*]
    } else { // ray looking at the sky
        result = 0;//float3(0.0);
    }
    return result;
}

// direct sun light for ray x+tv, when sun in direction s (=L0)
float3 sunColor(float3 x, float t, float3 v, float3 s, float r, float mu) {
    if (t > 0.0) {
       return 0;//float3(0.0);
    } else {
        float3 transmittance =  float3(1,1,1); // T(x,xo)
        float isun = step(cos(M_PI / 180.0), dot(v, s)) * ISun* ISun; // Lsun
        return transmittance * isun; // Eq (9)
    }
}


float3 get_sky(float3 p, float3 v, float t)
{
    float3 x = Scaler * p + float3(0, Rg, 0);
  


    
    float r = 0;
    float mu = 0;
    
    float3 attenuation = 0;
    float3 s = light_dir;
    float3 inscatterColor = inscatter(x, t, v, s, r, mu, attenuation);
    float3 sunColor_ = sunColor(x, t, v, s, r, mu);

    return 1;// sunColor_ + 1 * inscatterColor;
}

	float4 PS(quad_output i) : SV_TARGET0
	{
	
		return float4(get_sky(float3(0,100,0),normalize(i.tc),0),1);

	}

#endif