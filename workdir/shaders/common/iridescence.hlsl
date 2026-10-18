#ifndef IRIDESCENCE_HLSL
#define IRIDESCENCE_HLSL

// Thin-film interference on a specular surface (anodised metal, oil, soap):
// Fresnel of a conductor or dielectric under a film of thickness d and index n.
//
// Belcour & Barla, "A Practical Extension to Microfacet Theory for the Modeling
// of Varying Iridescence", SIGGRAPH 2017: the Airy summation of the first two
// reflection orders, with the spectral response folded into a Gaussian fit of the
// CIE matching functions. This is the formulation glTF's KHR_materials_iridescence
// uses, so the same film parameters look the same.

static const float IRI_PI = 3.14159265;

float iri_ior_to_f0(float transmitted, float incident)
{
    float r = (transmitted - incident) / (transmitted + incident);
    return r * r;
}

float3 iri_ior_to_f0(float3 transmitted, float incident)
{
    float3 r = (transmitted - incident) / (transmitted + incident);
    return r * r;
}

float3 iri_f0_to_ior(float3 f0)
{
    float3 s = sqrt(clamp(f0, 0.0, 0.9999));
    return (1.0 + s) / (1.0 - s);
}

float3 iri_schlick(float3 f0, float cos_theta)
{
    float x = saturate(1.0 - cos_theta);
    float x2 = x * x;
    return f0 + (1.0 - f0) * (x2 * x2 * x);
}

float iri_schlick(float f0, float cos_theta)
{
    float x = saturate(1.0 - cos_theta);
    float x2 = x * x;
    return f0 + (1.0 - f0) * (x2 * x2 * x);
}

// The optical path difference's spectral weight, as linear Rec.709 RGB.
float3 iri_sensitivity(float opd, float3 shift)
{
    float phase = 2.0 * IRI_PI * opd * 1.0e-9;
    float3 val = float3(5.4856e-13, 4.4201e-13, 5.2481e-13);
    float3 pos = float3(1.6810e+06, 1.7953e+06, 2.2084e+06);
    float3 var = float3(4.3278e+09, 9.3046e+09, 6.6121e+09);

    float3 xyz = val * sqrt(2.0 * IRI_PI * var) * cos(pos * phase + shift) * exp(-phase * phase * var);
    xyz.x += 9.7470e-14 * sqrt(2.0 * IRI_PI * 4.5282e+09) * cos(2.2399e+06 * phase + shift.x) * exp(-4.5282e+09 * phase * phase);
    xyz /= 1.0685e-7;

    return float3(
         3.2404542 * xyz.x - 1.5371385 * xyz.y - 0.4985314 * xyz.z,
        -0.9692660 * xyz.x + 1.8760108 * xyz.y + 0.0415560 * xyz.z,
         0.0556434 * xyz.x - 0.2040259 * xyz.y + 1.0572252 * xyz.z);
}

// Fresnel reflectance of base (F0 at normal incidence) under the film.
// thickness in nanometres, film_ior the film's index of refraction.
float3 iridescent_fresnel(float cos_theta, float thickness, float film_ior, float3 base_f0)
{
    const float outside = 1.0;

    // No film: the bare interface. (The interference sum does not vanish exactly
    // at zero thickness, so this is not left to the formula.)
    if (thickness < 0.5)
        return iri_schlick(base_f0, cos_theta);

    // A film thinner than a few nm is no film: blend to the bare interface.
    float ior = lerp(outside, film_ior, smoothstep(0.0, 0.03, thickness));

    float sin2 = (outside / ior) * (outside / ior) * (1.0 - cos_theta * cos_theta);
    float cos2sq = 1.0 - sin2;

    // Total internal reflection inside the film.
    if (cos2sq < 0.0)
        return float3(1, 1, 1);

    float cos2 = sqrt(cos2sq);

    float r0 = iri_ior_to_f0(ior, outside);
    float r12 = iri_schlick(r0, cos_theta);
    float t121 = 1.0 - r12;
    float phi12 = ior < outside ? IRI_PI : 0.0;
    float phi21 = IRI_PI - phi12;

    float3 base_ior = iri_f0_to_ior(base_f0);
    float3 r1 = iri_ior_to_f0(base_ior, ior);
    float3 r23 = iri_schlick(r1, cos2);

    float3 phi23 = float3(base_ior.x < ior ? IRI_PI : 0.0, base_ior.y < ior ? IRI_PI : 0.0, base_ior.z < ior ? IRI_PI : 0.0);

    float opd = 2.0 * ior * thickness * cos2;
    float3 phi = float3(phi21, phi21, phi21) + phi23;

    float3 r123 = clamp(r12 * r23, 1e-5, 0.9999);
    float3 root = sqrt(r123);
    float3 rs = (t121 * t121) * r23 / (1.0 - r123);

    // Zeroth order, then the first two interference orders.
    float3 result = r12 + rs;
    float3 cm = rs - t121;

    [unroll]
    for (int m = 1; m <= 2; m++)
    {
        cm *= root;
        result += cm * (2.0 * iri_sensitivity(m * opd, m * phi));
    }

    return max(result, 0.0);
}

// The F0 that gives the iridescent reflectance back through a plain Schlick
// Fresnel at this angle (f90 = 1): lets the split-sum environment BRDF, which is
// parameterised by F0, carry the film.
float3 iridescent_f0(float cos_theta, float thickness, float film_ior, float3 base_f0)
{
    float3 f = iridescent_fresnel(cos_theta, thickness, film_ior, base_f0);
    float x = saturate(1.0 - cos_theta);
    float x5 = x * x * x * x * x;

    return saturate((f - x5) / max(1.0 - x5, 1e-3));
}

#endif
