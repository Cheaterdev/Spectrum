module Graphics:Camera;
import Core;

void camera::update(float2 offset )
{
	auto jitter_mat = mat4x4::translation(vec3{ offset, 0 });
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
	params.viewProj = view_proj_mat;
	params.view = view_mat;
	params.proj = proj_jittered;
	params.position = float4(pos,0);
	params.direction = float4(_dir.normalize(),0);
	params.invViewProj = inv_view_proj_mat;
	(params.invProj = proj_jittered).inverse();
	(params.invView = view_mat).inverse();
	mat4x4 test = view_proj_mat * inv_view_proj_mat;
	calculate(inv_view_proj_mat);
	vec3 right = vec3::cross(params.direction.xyz, _up);
	res_up = vec3::cross(params.direction.xyz, right);
	params.jitter = { 0, 0,0,0 };// offset;
	

	for (int i = 0; i < 6; i++)
		params.frustum.planes[i] = Frustum::GetFrustumPlane(planes(i));


	camera_cb.prev = camera_cb.current;
	camera_cb.current = params;
}
