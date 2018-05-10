#include "pch.h"
void camera::update(float2 offset )
{
	auto jitter_mat = mat4x4::translation({ offset, 0 });
	auto proj_jittered = proj_mat;// *jitter_mat;

	vec3 _dir = target - position;// (0, 0, 1);
	vec3 _up (0, 1, 0);

	_dir = eye_rot.rotate(_dir);
	_up = eye_rot.rotate(_up);

	vec3 pos = position +this->offset;// (0, 1, 0);

	//_dir = target - position;
	view_mat.look_at(pos, pos + _dir, _up);
	view_proj_mat = view_mat * proj_jittered;
	(inv_view_proj_mat = view_proj_mat).inverse();
	params.view_proj = view_proj_mat;
	params.view = view_mat;
	params.proj = proj_jittered;
	params.position = pos;
	params.direction = _dir.normalize();
	params.inv_view_proj = inv_view_proj_mat;
	(params.inv_proj = proj_jittered).inverse();
	(params.inv_view = view_mat).inverse();
	mat4x4 test = view_proj_mat * inv_view_proj_mat;
	calculate(inv_view_proj_mat);
	vec3 right = vec3::cross(params.direction, _up);
	res_up = vec3::cross(params.direction, right);
	params.jitter = { 0, 0 };// offset;
	const_buffer.data().prev = const_buffer.data().current;
	const_buffer.data().current = params;
}
