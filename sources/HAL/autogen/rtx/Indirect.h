#pragma once
struct Indirect: public RaytraceRaygen<Indirect>
{
	static const constexpr uint ID = 2;
	static const constexpr std::string_view shader = "shaders\\raytracing.hlsl";
	static const constexpr std::wstring_view raygen = L"MyRaygenShader";
};
