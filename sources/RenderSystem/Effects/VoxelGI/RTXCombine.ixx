// Composites ReflectionRTX/IndirectRTX/ShadowRTX onto ResultTexture -- the
// DLSS-RR-active counterpart to ReflCombine. No new exports of its own, see
// voxel.prism's PassNode RTXCombine doc comment.
export module Graphics:RTXCombine;

import HAL;
