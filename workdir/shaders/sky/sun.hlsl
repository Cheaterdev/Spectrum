#ifndef SKY_SUN_HLSL
#define SKY_SUN_HLSL

// Shared by the sky's sun disk and the lens flare's occlusion sampling, so the
// flare tests exactly the disk the sky draws.

// Mean angular radius of the Sun seen from Earth (0.2666 deg), times an
// artistic enlargement: at true size the disk is only a few pixels at render
// resolution.
static const float SunSizeScale = 2.5;
static const float SunAngularRadius = 0.004653 * SunSizeScale;

#endif
