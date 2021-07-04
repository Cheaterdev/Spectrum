#pragma once
struct ShadowPass: public RaytracePass<ShadowPass>
{
	using Payload = Table::ShadowPayload;
	static const constexpr UINT ID = 0;
	static const constexpr std::string_view shader = "shaders\\raytracing.hlsl";
	static const constexpr std::wstring_view name = L"ShadowPass_GROUP";
	static const constexpr std::wstring_view hit_name = L"ShadowClosestHitShader";
	static const constexpr std::wstring_view miss_name = L"ShadowMissShader";
	static const constexpr bool per_material = false;
};
