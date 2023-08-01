#pragma once
struct Shadow: public RaytraceRaygen<Shadow>
{
	static const constexpr uint ID = 0;
	static const constexpr std::string_view shader = "shaders\\raytracing.hlsl";
	static const constexpr std::wstring_view raygen = L"ShadowRaygenShader";
};
