#include "pch.h"

void MathTest()
{
	mat4x4 m1, m2, m3;
	m1 = m2*m3;
	matrix<matrix_data_t<4, 4, float>> tt, a, b;
	tt.ZeroMatrix();
	tt[0][0] = 1;
	tt[1][0] = 2;
	a = tt;
	b = tt;
	tt = a + b;
	tt = a - b;
	tt = -a;
	tt = -a * 4;
	tt = -5 * a * 4;
	tt = -a * 4;
	Vector<vector_data_t<float, 4>> k;
	vec3 kk;
	k = kk;
	k += kk;
	k = k + kk;
	//tt = tt * 5.0f;
	//tt = 5.0f * tt;
}