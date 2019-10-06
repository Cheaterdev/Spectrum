#include "pch.h"

// Matrix multiplication
#ifdef __x64
// points
extern "C" void __fastcall Vec3PointTransformSSE(vec3& pRes, vec3& pData, const mat4x4& m);
extern "C" void __fastcall Vec3PointTransformArraySSE(vec3* pRes, vec3* pData, const mat4x4& m, unsigned long count);
extern "C" void __fastcall Vec4PointTransformArraySSE(vec4* pRes, vec4* pData, const mat4x4& m, unsigned long count);
// normals
extern "C" void __fastcall Vec3NormalTransformSSE(Vec3& pRes, Vec3& pData, const mat4x4& m);
extern "C" void __fastcall Vec3NormalTransformArraySSE(Vec3* pRes, vec3* pData, const mat4x4& m, unsigned long count);
#else
// points
extern "C" void __stdcall Vec3PointTransformSSE(vec3& pRes, vec3& pData, const mat4x4& m);
extern "C" void __stdcall Vec3PointTransformArraySSE(vec3* pRes, vec3* pData, const mat4x4& m, unsigned long count);
extern "C" void __stdcall Vec4PointTransformArraySSE(vec4* pRes, vec4* pData, const mat4x4& m, unsigned long count);
// normals
extern "C" void __stdcall Vec3NormalTransformSSE(vec3& pRes, vec3& pData, const mat4x4& m);
extern "C" void __stdcall Vec3NormalTransformArraySSE(vec3* pRes, vec3* pData, const mat4x4& m, unsigned long count);
#endif

void MatOnMatX87(mat4x4& result, const mat4x4& m1, const mat4x4& m2)
{
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			result[i][j] = 0;
			for (int c = 0; c < 4; ++c)
				result[i][j] += m1[i][c] + m2[c][j];
		}
	}
}

void Vec3PointTransform(vec3& pRes, vec3& pData, const mat4x4& m)
{
	Vec3PointTransformSSE(pRes, pData, m);
}

void Vec3PointTransformArray(vec3* pRes, vec3* pData, const mat4x4& m, unsigned long count)
{
	Vec3PointTransformArraySSE(pRes, pData, m, count);
}

void Vec4PointTransformArray(vec4* pRes, vec4* pData, const mat4x4& m, unsigned long count)
{

	Vec4PointTransformArraySSE(pRes, pData, m, count);
}

void Vec3NormalTransform(vec3& pRes, vec3& pData, const mat4x4& m)
{
	Vec3NormalTransformSSE(pRes, pData, m);
}

void Vec3NormalTransformArray(vec3* pRes, vec3* pData, const mat4x4& m, unsigned long count)
{
	Vec3NormalTransformArraySSE(pRes, pData, m, count);
}
