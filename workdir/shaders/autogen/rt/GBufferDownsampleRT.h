#pragma once
struct GBufferDownsampleRT
{
	float depth: SV_Target0;
	float4 color: SV_Target1;
};
