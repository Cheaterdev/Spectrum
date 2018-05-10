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
	assert((((int)&m) & 0xF) == 0);
	Vec3PointTransformSSE(pRes, pData, m);
}

void Vec3PointTransformArray(vec3* pRes, vec3* pData, const mat4x4& m, unsigned long count)
{
	assert((((int)&m) & 0xF) == 0);
	Vec3PointTransformArraySSE(pRes, pData, m, count);
}

void Vec4PointTransformArray(vec4* pRes, vec4* pData, const mat4x4& m, unsigned long count)
{
	assert(((int)&m & 0xF) == 0);
	assert(((int)pRes & 0xF) == 0);
	assert(((int)pData & 0xF) == 0);
	Vec4PointTransformArraySSE(pRes, pData, m, count);
}

void Vec3NormalTransform(vec3& pRes, vec3& pData, const mat4x4& m)
{
	assert(((int)&m & 0xF) == 0);
	Vec3NormalTransformSSE(pRes, pData, m);
}

void Vec3NormalTransformArray(vec3* pRes, vec3* pData, const mat4x4& m, unsigned long count)
{
	assert(((int)&m & 0xF) == 0);
	Vec3NormalTransformArraySSE(pRes, pData, m, count);
}

/*
mat4x4 mat4x4::operator*(const mat4x4& m2) const
{
mat4x4 r;
MatOnMatX87(r, *this, m2);
// need to align
//Vec4PointTransformArray((Vec4*)&r, (Vec4*)this, m2, 4);
return r;
}*/
/*
mat4x4& mat4x4::operator*=(const mat4x4& m2)
{
mat4x4 r;
MatOnMatX87(r, *this, m2);
*this = r;
return *this;
}
*/