#pragma once
struct GBuffer
{
	float4 albedo: SV_Target0;
	float4 normals: SV_Target1;
	float4 specular: SV_Target2;
	float2 motion: SV_Target3;
};
