#pragma once
struct MaterialPass: public RaytracePass<MaterialPass>
{
	using Payload = Table::RayPayload;
	using LocalData =  Slots::MaterialInfo;
	static const constexpr UINT ID = 1;
	static const constexpr std::string_view shader = "shaders\\raytracing.hlsl";
	static const constexpr std::wstring_view name = L"MaterialPass_GROUP";
	static const constexpr std::wstring_view hit_name = L"MyClosestHitShader";
	static const constexpr std::wstring_view miss_name = L"MyMissShader";
	static const constexpr bool per_material = true;
};
