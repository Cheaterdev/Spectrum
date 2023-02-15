#include "autogen/BlueNoise.h"
#define GOLDEN_RATIO                       1.61803398875f

// Blue Noise Sampler by Eric Heitz. Returns a value in the range [0, 1].
float SampleRandomNumber(uint pixel_i, uint pixel_j, uint sample_index, uint sample_dimension) {
    // Wrap arguments
    pixel_i = pixel_i & 127u;
    pixel_j = pixel_j & 127u;
    sample_index = sample_index & 255u;
    sample_dimension = sample_dimension & 255u;

    // xor index based on optimized ranking
    const uint ranked_sample_index = sample_index ^ GetBlueNoise().GetRanking_tile_buffer()[sample_dimension + (pixel_i + pixel_j * 128u) * 8u];

    // Fetch value in sequence
    uint value = GetBlueNoise().GetSobol_buffer()[sample_dimension + ranked_sample_index * 256u];

    // If the dimension is optimized, xor sequence value based on optimized scrambling
    value = value ^ GetBlueNoise().GetScrambling_tile_buffer()[(sample_dimension % 8u) + (pixel_i + pixel_j * 128u) * 8u];

    // Convert to float and return
    return (value + 0.5f) / 256.0f;
}

float2 SampleRandomVector2D(uint2 pixel) {
    float2 u = float2(
        fmod(SampleRandomNumber(pixel.x, pixel.y, 0, 0u) + (GetBlueNoise().GetFrame_index() & 0xFFu) * GOLDEN_RATIO, 1.0f),
        fmod(SampleRandomNumber(pixel.x, pixel.y, 0, 1u) + (GetBlueNoise().GetFrame_index() & 0xFFu) * GOLDEN_RATIO, 1.0f));
    return u;
}

[numthreads(8, 8, 1)]
void CS(uint2 dispatch_thread_id : SV_DispatchThreadID) {
    GetBlueNoise().GetBlue_noise_texture()[dispatch_thread_id] = SampleRandomVector2D(dispatch_thread_id);
}