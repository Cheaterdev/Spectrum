// RTX-only reflection pass, for DLSS-RR's consumption. No new exports of its
// own -- see voxel.sig's PassNode ReflectionRTX doc comment for why this is
// separate from ScreenReflection.
export module Graphics:ReflectionRTX;

import HAL;
