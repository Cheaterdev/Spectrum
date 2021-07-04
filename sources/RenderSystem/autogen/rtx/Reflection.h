#pragma once
struct Reflection: public RaytraceRaygen<Reflection>
{
	static const constexpr UINT ID = 1;
	static const constexpr std::string_view shader = "shaders\\raytracing.hlsl";
	static const constexpr std::wstring_view raygen = L"MyRaygenShaderReflection";
};
