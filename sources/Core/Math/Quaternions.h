#pragma once
//class mat4x4;
class quat : public vec4
{
public:
	quat() {}
	quat(float x, float y, float z, float w = 1);
	quat(vec3 dir, float angle);
	quat(vec3 v);
	quat(vec4 v);

	float norm() const { return x*x + y*y + z*z + w*w; }
	float magnitude() const { return sqrt(norm()); }
	quat& conjugate() { x *= -1; y *= -1; z *= -1; return *this; }
	quat& inverse()
	{
		conjugate();

		return *this;
	}

	void identity() { x = y = z = 0; w = 1; }
	quat operator*(const quat& q) const;
	quat& operator*=(const quat& q);
//	void toMatrix(mat4x4& m) const;

	void to_axis_angle(vec3 &axis, float &angle) const
	{

		angle = acosf(w);

		// pre-compute to save time
		float sinf_theta_inv = 1.0f / sinf(angle);

		// now the vector
		axis.x = x * sinf_theta_inv;
		axis.y = y * sinf_theta_inv;
		axis.z = z * sinf_theta_inv;

		// multiply by 2
		angle *= 2;
	}

	//! rotates v by this quat (quat must be unit)
	vec3 rotate(const vec3 &vec)
	{
		float num = x * 2.f;
		float num2 = y * 2.f;
		float num3 = z * 2.f;
		float num4 = x * num;
		float num5 = y * num2;
		float num6 = z * num3;
		float num7 = x * num2;
		float num8 = x * num3;
		float num9 = y * num3;
		float num10 =w * num;
		float num11 = w * num2;
		float num12 = w * num3;

		float tx = (1.f - (num5 + num6)) * vec.x + (num7 + num12) * vec.y + (num8 - num11) * vec.z;
		float ty = (num7 - num12) * vec.x + (1.f - (num4 + num6)) * vec.y + (num9 + num10) * vec.z;
		float tz = (num8 + num11) * vec.x + (num9 - num10) * vec.y + (1.f - (num4 + num5)) * vec.z;
		//vec3 t;
		//t.set(tx, ty, tz);
		return { tx, ty, tz };
	}


};