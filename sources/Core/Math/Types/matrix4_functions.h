static 	matrix translation(vec3 dpos) requires(matrix_type::N == 4 && matrix_type::M == 4)
{
	matrix r;
	r.a11 = r.a22 = r.a33 = r.a44 = 1.0f;
	r.a12 = r.a13 = r.a14 = 0.0f;
	r.a21 = r.a23 = r.a24 = 0.0f;
	r.a31 = r.a32 = r.a34 = 0.0f;
	r.a41 = dpos.x;
	r.a42 = dpos.y;
	r.a43 = dpos.z;
	return r;
}

// Transformation matrices

static 	matrix rotationX(float angle) requires(matrix_type::N == 4 && matrix_type::M == 4)
{
	matrix r;
	float cosA = cos(angle);
	float sinA = sin(angle);
	r.a11 = r.a44 = 1.0f;
	r.a12 = r.a13 = r.a14 = 0.0f;
	r.a21 = r.a24 = 0.0f;
	r.a31 = r.a34 = 0.0f;
	r.a41 = r.a42 = r.a43 = 0.0f;
	r.a22 = r.a33 = cosA;
	r.a23 = sinA;
	r.a32 = -sinA;
	return r;
}


static matrix rotationY(float angle) requires(matrix_type::N == 4 && matrix_type::M == 4)
{
	matrix r;
	float cosA = cos(angle);
	float sinA = sin(angle);
	r.a22 = r.a44 = 1.0f;
	r.a12 = r.a14 = 0.0f;
	r.a21 = r.a23 = r.a24 = 0.0f;
	r.a32 = r.a34 = 0.0f;
	r.a41 = r.a42 = r.a43 = 0.0f;
	r.a11 = r.a33 = cosA;
	r.a13 = -sinA;
	r.a31 = sinA;
	return r;
}

static matrix rotationZ(float angle) requires(matrix_type::N == 4 && matrix_type::M == 4)
{
	matrix r;
	float cosA = cos(angle);
	float sinA = sin(angle);
	r.a33 = r.a44 = 1.0f;
	r.a13 = r.a14 = 0.0f;
	r.a23 = r.a24 = 0.0f;
	r.a31 = r.a32 = r.a34 = 0.0f;
	r.a41 = r.a42 = r.a43 = 0.0f;
	r.a11 = r.a22 = cosA;
	r.a12 = sinA;
	r.a21 = -sinA;
	return r;
}

matrix& identity() requires(matrix_type::N == 4 && matrix_type::M == 4)
{
	matrix_type::a11 = matrix_type::a22 = matrix_type::a33 = matrix_type::a44 = 1;
	matrix_type::a12 = matrix_type::a13 = matrix_type::a14 = 0;
	matrix_type::a21 = matrix_type::a23 = matrix_type::a24 = 0;
	matrix_type::a31 = matrix_type::a32 = matrix_type::a34 = 0;
	matrix_type::a41 = matrix_type::a42 = matrix_type::a43 = 0;
	return *this;
}

Format det() const requires(matrix_type::N == 4 && matrix_type::M == 4)
{
	Format d = 0;
	d += matrix_type::a11 * (matrix_type::a22 * matrix_type::a33 * matrix_type::a44 + matrix_type::a32 * matrix_type::a43 * matrix_type::a24 + matrix_type::a23 * matrix_type::a34 * matrix_type::a42 - (matrix_type::a24 * matrix_type::a33 * matrix_type::a42 + matrix_type::a32 * matrix_type::a23 * matrix_type::a44 + matrix_type::a43 * matrix_type::a34 * matrix_type::a22));
	d -= matrix_type::a12 * (matrix_type::a21 * matrix_type::a33 * matrix_type::a44 + matrix_type::a31 * matrix_type::a43 * matrix_type::a24 + matrix_type::a23 * matrix_type::a34 * matrix_type::a41 - (matrix_type::a24 * matrix_type::a33 * matrix_type::a41 + matrix_type::a31 * matrix_type::a23 * matrix_type::a44 + matrix_type::a43 * matrix_type::a34 * matrix_type::a21));
	d += matrix_type::a13 * (matrix_type::a21 * matrix_type::a32 * matrix_type::a44 + matrix_type::a31 * matrix_type::a42 * matrix_type::a24 + matrix_type::a22 * matrix_type::a34 * matrix_type::a41 - (matrix_type::a24 * matrix_type::a32 * matrix_type::a41 + matrix_type::a31 * matrix_type::a22 * matrix_type::a44 + matrix_type::a42 * matrix_type::a34 * matrix_type::a21));
	d -= matrix_type::a14 * (matrix_type::a21 * matrix_type::a32 * matrix_type::a43 + matrix_type::a31 * matrix_type::a42 * matrix_type::a23 + matrix_type::a22 * matrix_type::a33 * matrix_type::a41 - (matrix_type::a23 * matrix_type::a32 * matrix_type::a41 + matrix_type::a31 * matrix_type::a22 * matrix_type::a43 + matrix_type::a42 * matrix_type::a33 * matrix_type::a21));
	return d;
}

bool inverse() requires(matrix_type::N == 4 && matrix_type::M == 4)
{
	Format d = det();
	d = Format(1) / d;

	matrix m;
	m.a11 = matrix_type::a22 * matrix_type::a33 * matrix_type::a44 + matrix_type::a32 * matrix_type::a43 * matrix_type::a24 + matrix_type::a23 * matrix_type::a34 * matrix_type::a42 - matrix_type::a24 * matrix_type::a33 * matrix_type::a42 - matrix_type::a32 * matrix_type::a23 * matrix_type::a44 - matrix_type::a43 * matrix_type::a34 * matrix_type::a22;
	m.a21 = matrix_type::a24 * matrix_type::a33 * matrix_type::a41 + matrix_type::a31 * matrix_type::a23 * matrix_type::a44 + matrix_type::a43 * matrix_type::a34 * matrix_type::a21 - matrix_type::a21 * matrix_type::a33 * matrix_type::a44 - matrix_type::a31 * matrix_type::a43 * matrix_type::a24 - matrix_type::a23 * matrix_type::a34 * matrix_type::a41;
	m.a31 = matrix_type::a21 * matrix_type::a32 * matrix_type::a44 + matrix_type::a31 * matrix_type::a42 * matrix_type::a24 + matrix_type::a22 * matrix_type::a34 * matrix_type::a41 - matrix_type::a24 * matrix_type::a32 * matrix_type::a41 - matrix_type::a31 * matrix_type::a22 * matrix_type::a44 - matrix_type::a42 * matrix_type::a34 * matrix_type::a21;
	m.a41 = matrix_type::a23 * matrix_type::a32 * matrix_type::a41 + matrix_type::a31 * matrix_type::a22 * matrix_type::a43 + matrix_type::a42 * matrix_type::a33 * matrix_type::a21 - matrix_type::a21 * matrix_type::a32 * matrix_type::a43 - matrix_type::a31 * matrix_type::a42 * matrix_type::a23 - matrix_type::a22 * matrix_type::a33 * matrix_type::a41;
	m.a12 = matrix_type::a14 * matrix_type::a33 * matrix_type::a42 + matrix_type::a32 * matrix_type::a13 * matrix_type::a44 + matrix_type::a43 * matrix_type::a34 * matrix_type::a12 - matrix_type::a12 * matrix_type::a33 * matrix_type::a44 - matrix_type::a32 * matrix_type::a43 * matrix_type::a14 - matrix_type::a13 * matrix_type::a34 * matrix_type::a42;
	m.a22 = matrix_type::a11 * matrix_type::a33 * matrix_type::a44 + matrix_type::a31 * matrix_type::a43 * matrix_type::a14 + matrix_type::a13 * matrix_type::a34 * matrix_type::a41 - matrix_type::a14 * matrix_type::a33 * matrix_type::a41 - matrix_type::a31 * matrix_type::a13 * matrix_type::a44 - matrix_type::a43 * matrix_type::a34 * matrix_type::a11;
	m.a32 = matrix_type::a14 * matrix_type::a32 * matrix_type::a41 + matrix_type::a31 * matrix_type::a12 * matrix_type::a44 + matrix_type::a42 * matrix_type::a34 * matrix_type::a11 - matrix_type::a11 * matrix_type::a32 * matrix_type::a44 - matrix_type::a31 * matrix_type::a42 * matrix_type::a14 - matrix_type::a12 * matrix_type::a34 * matrix_type::a41;
	m.a42 = matrix_type::a11 * matrix_type::a32 * matrix_type::a43 + matrix_type::a31 * matrix_type::a42 * matrix_type::a13 + matrix_type::a12 * matrix_type::a33 * matrix_type::a41 - matrix_type::a13 * matrix_type::a32 * matrix_type::a41 - matrix_type::a31 * matrix_type::a12 * matrix_type::a43 - matrix_type::a42 * matrix_type::a33 * matrix_type::a11;
	m.a13 = matrix_type::a12 * matrix_type::a23 * matrix_type::a44 + matrix_type::a22 * matrix_type::a43 * matrix_type::a14 + matrix_type::a13 * matrix_type::a24 * matrix_type::a42 - matrix_type::a14 * matrix_type::a23 * matrix_type::a42 - matrix_type::a22 * matrix_type::a13 * matrix_type::a44 - matrix_type::a43 * matrix_type::a24 * matrix_type::a12;
	m.a23 = matrix_type::a14 * matrix_type::a23 * matrix_type::a41 + matrix_type::a21 * matrix_type::a13 * matrix_type::a44 + matrix_type::a43 * matrix_type::a24 * matrix_type::a11 - matrix_type::a11 * matrix_type::a23 * matrix_type::a44 - matrix_type::a21 * matrix_type::a43 * matrix_type::a14 - matrix_type::a13 * matrix_type::a24 * matrix_type::a41;
	m.a33 = matrix_type::a11 * matrix_type::a22 * matrix_type::a44 + matrix_type::a21 * matrix_type::a42 * matrix_type::a14 + matrix_type::a12 * matrix_type::a24 * matrix_type::a41 - matrix_type::a14 * matrix_type::a22 * matrix_type::a41 - matrix_type::a21 * matrix_type::a12 * matrix_type::a44 - matrix_type::a42 * matrix_type::a24 * matrix_type::a11;
	m.a43 = matrix_type::a13 * matrix_type::a22 * matrix_type::a41 + matrix_type::a21 * matrix_type::a12 * matrix_type::a43 + matrix_type::a42 * matrix_type::a23 * matrix_type::a11 - matrix_type::a11 * matrix_type::a22 * matrix_type::a43 - matrix_type::a21 * matrix_type::a42 * matrix_type::a13 - matrix_type::a12 * matrix_type::a23 * matrix_type::a41;
	m.a14 = matrix_type::a14 * matrix_type::a23 * matrix_type::a32 + matrix_type::a22 * matrix_type::a13 * matrix_type::a34 + matrix_type::a33 * matrix_type::a24 * matrix_type::a12 - matrix_type::a12 * matrix_type::a23 * matrix_type::a34 - matrix_type::a22 * matrix_type::a33 * matrix_type::a14 - matrix_type::a13 * matrix_type::a24 * matrix_type::a32;
	m.a24 = matrix_type::a11 * matrix_type::a23 * matrix_type::a34 + matrix_type::a21 * matrix_type::a33 * matrix_type::a14 + matrix_type::a13 * matrix_type::a24 * matrix_type::a31 - matrix_type::a14 * matrix_type::a23 * matrix_type::a31 - matrix_type::a21 * matrix_type::a13 * matrix_type::a34 - matrix_type::a33 * matrix_type::a24 * matrix_type::a11;
	m.a34 = matrix_type::a14 * matrix_type::a22 * matrix_type::a31 + matrix_type::a21 * matrix_type::a12 * matrix_type::a34 + matrix_type::a32 * matrix_type::a24 * matrix_type::a11 - matrix_type::a11 * matrix_type::a22 * matrix_type::a34 - matrix_type::a21 * matrix_type::a32 * matrix_type::a14 - matrix_type::a12 * matrix_type::a24 * matrix_type::a31;
	m.a44 = matrix_type::a11 * matrix_type::a22 * matrix_type::a33 + matrix_type::a21 * matrix_type::a32 * matrix_type::a13 + matrix_type::a12 * matrix_type::a23 * matrix_type::a31 - matrix_type::a13 * matrix_type::a22 * matrix_type::a31 - matrix_type::a21 * matrix_type::a12 * matrix_type::a33 - matrix_type::a32 * matrix_type::a23 * matrix_type::a11;

	Format* data = (Format*)&m;

	for (int i = 0; i < 16; ++i)
		data[i] *= d;

	*this = m;
	return true;
}
matrix& transpose()requires(matrix_type::N == 4 && matrix_type::M == 4)
{
	// row 1
	swap(matrix_type::a12, matrix_type::a21);
	swap(matrix_type::a13, matrix_type::a31);
	swap(matrix_type::a14, matrix_type::a41);
	// row 2
	swap(matrix_type::a23, matrix_type::a32);
	swap(matrix_type::a24, matrix_type::a42);
	// row 3
	swap(matrix_type::a34, matrix_type::a43);
	return *this;
}


matrix& rotationAxis(vec3 dir, float angle) requires(matrix_type::N == 4 && matrix_type::M == 4)
{
	quat q(dir, angle);
	//  q.toMatrix(*this);
	return *this;
}

matrix& scaling(float scale) requires(matrix_type::N == 4 && matrix_type::M == 4)
{
	return scaling(scale, scale, scale);
}
matrix& scaling(vec3 scale) requires(matrix_type::N == 4 && matrix_type::M == 4)
{
	return scaling(scale.x, scale.y, scale.z);
}
matrix& scaling(float scaleX, float scaleY, float scaleZ)requires(matrix_type::N == 4 && matrix_type::M == 4)
{
	matrix_type::a11 = scaleX;
	matrix_type::a22 = scaleY;
	matrix_type::a33 = scaleZ;
	matrix_type::a44 = 1.0f;
	matrix_type::a12 = matrix_type::a13 = matrix_type::a14 = 0.0f;
	matrix_type::a21 = matrix_type::a23 = matrix_type::a24 = 0.0f;
	matrix_type::a31 = matrix_type::a32 = matrix_type::a34 = 0.0f;
	matrix_type::a41 = matrix_type::a42 = matrix_type::a43 = 0.0f;
	return *this;
}
matrix& look_at(const vec3& eye, const vec3& at, const vec3& up)requires(matrix_type::N == 4 && matrix_type::M == 4)
{
	vec3 zaxis = (at - eye).normalize();
	vec3 xaxis = vec3::cross(up, zaxis).normalize();
	vec3 yaxis = vec3::cross(zaxis, xaxis);
	matrix_type::a11 = xaxis.x; matrix_type::a12 = yaxis.x; matrix_type::a13 = zaxis.x; matrix_type::a14 = 0;
	matrix_type::a21 = xaxis.y; matrix_type::a22 = yaxis.y; matrix_type::a23 = zaxis.y; matrix_type::a24 = 0;
	matrix_type::a31 = xaxis.z; matrix_type::a32 = yaxis.z; matrix_type::a33 = zaxis.z; matrix_type::a34 = 0;
	matrix_type::a41 = -vec3::dot(xaxis, eye);
	matrix_type::a42 = -vec3::dot(yaxis, eye);
	matrix_type::a43 = -vec3::dot(zaxis, eye);
	matrix_type::a44 = 1;
	return *this;
}
matrix& perspective(float fovy, float aspect, float zn, float zf) requires(matrix_type::N == 4 && matrix_type::M == 4)
{
	float yScale = 1 / tan(0.5f * fovy);
	float xScale = yScale / aspect;
	matrix_type::a11 = xScale; matrix_type::a12 = 0; matrix_type::a13 = 0; matrix_type::a14 = 0;
	matrix_type::a21 = 0; matrix_type::a22 = yScale; matrix_type::a23 = 0; matrix_type::a24 = 0;
	matrix_type::a31 = 0; matrix_type::a32 = 0; matrix_type::a33 = zf / (zf - zn); matrix_type::a34 = 1;
	matrix_type::a41 = 0; matrix_type::a42 = 0; matrix_type::a43 = -zn * matrix_type::a33; matrix_type::a44 = 0;
	return *this;
}
matrix& orthographic(float l, float r, float t, float b, float zn, float zf) requires(matrix_type::N == 4 && matrix_type::M == 4)
{
	/*
	2/(r-l)      0            0           0
	0            2/(t-b)      0           0
	0            0            1/(zf-zn)   0
	(l+r)/(l-r)  (t+b)/(b-t)  zn/(zn-zf)  1
	*/
	matrix_type::a11 = 2 / (r - l); matrix_type::a12 = 0; matrix_type::a13 = 0; matrix_type::a14 = 0;
	matrix_type::a21 = 0; matrix_type::a22 = -2 / (t - b); matrix_type::a23 = 0; matrix_type::a24 = 0;
	matrix_type::a31 = 0; matrix_type::a32 = 0; matrix_type::a33 = -1 / (zn - zf); matrix_type::a34 = 0;
	matrix_type::a41 = (l + r) / (l - r); matrix_type::a42 = -(t + b) / (b - t); matrix_type::a43 = zn / (zn - zf); matrix_type::a44 = 1;
	return *this;
}
matrix& orthogonalize() requires(matrix_type::N == 4 && matrix_type::M == 4)
{
	// Gram-Schmidt
	vec3 a(matrix_type::a11, matrix_type::a21, matrix_type::a31),
		b(matrix_type::a12, matrix_type::a22, matrix_type::a32),
		c(matrix_type::a13, matrix_type::a23, matrix_type::a33);
	float l;
	// a
	a.norm();
	// b
	l = vec3::dot(a, b);
	b -= l * a;
	b.norm();
	// c
	l = vec3::dot(a, c);
	c -= l * a;
	l = vec3::dot(b, c);
	c -= l * b;
	c.norm();
	matrix_type::a11 = a.x; matrix_type::a21 = a.y; matrix_type::a31 = a.z;
	matrix_type::a12 = b.x; matrix_type::a22 = b.y; matrix_type::a32 = b.z;
	matrix_type::a13 = c.x; matrix_type::a23 = c.y; matrix_type::a33 = c.z;
	return *this;
}

matrix& skew(vec3& v)requires(matrix_type::N == 4 && matrix_type::M == 4)
{
	matrix_type::a11 = 0; matrix_type::a12 = -v.z; matrix_type::a13 = v.y; matrix_type::a14 = 0;
	matrix_type::a21 = v.z; matrix_type::a22 = 0; matrix_type::a23 = -v.x; matrix_type::a24 = 0;
	matrix_type::a31 = -v.y; matrix_type::a32 = v.x; matrix_type::a33 = 0; matrix_type::a34 = 0;
	matrix_type::a41 = 0; matrix_type::a42 = 0; matrix_type::a43 = 0; matrix_type::a44 = 1;
	return *this;
}
matrix& skew(vec4& v) requires(matrix_type::N == 4 && matrix_type::M == 4)
{
	matrix_type::a11 = 0; matrix_type::a12 = -v.z; matrix_type::a13 = v.y; matrix_type::a14 = 0;
	matrix_type::a21 = v.z; matrix_type::a22 = 0; matrix_type::a23 = -v.x; matrix_type::a24 = 0;
	matrix_type::a31 = -v.y; matrix_type::a32 = v.x; matrix_type::a33 = 0; matrix_type::a34 = 0;
	matrix_type::a41 = 0; matrix_type::a42 = 0; matrix_type::a43 = 0; matrix_type::a44 = 1;
	return *this;
}

matrix& skew(quat& q) requires(matrix_type::N == 4 && matrix_type::M == 4)
{
	matrix_type::a11 = -q.x; matrix_type::a12 = -q.y; matrix_type::a13 = -q.z; matrix_type::a14 = 0;
	matrix_type::a21 = q.w; matrix_type::a22 = q.z; matrix_type::a23 = -q.y; matrix_type::a24 = 0;
	matrix_type::a31 = -q.z; matrix_type::a32 = q.w; matrix_type::a33 = q.x; matrix_type::a34 = 0;
	matrix_type::a41 = q.y; matrix_type::a42 = -q.x; matrix_type::a43 = q.w; matrix_type::a44 = 1;
	return *this;
}
