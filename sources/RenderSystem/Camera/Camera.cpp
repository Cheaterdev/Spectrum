module Graphics:Camera;
import Core;
import "windows/windows.h";

box camera::frustrum_points::get_bounds_in(mat4x4 view_proj) const
{
    box res;

    for (int i = 0; i < 8; i++)
    {
        vec4 t = vec4(p[i], 1) * view_proj;
        vec3 p = vec3(t.xyz) / t.w;

        if (i == 0)
            res = box(p, p);
        else
        {
            vec3 mn = vec3::min(p, vec3(res.a));
            vec3 mx = vec3::max(p, vec3(res.b));
            res = box(mn, mx);
        }
    }

    return res;
}

vec3 camera::to_direction(vec2 local) const
{
    vec4 dir(local * 2 - vec2(1, 1), 1, 1);
    dir.y = -dir.y;
    dir = dir * get_inv_view_proj();
    vec3 res(dir.xyz / dir.w);
    res -= camera_cb.current.position.xyz;
    res.normalize();
    return res;
}

vec3 camera::to_direction(vec3 world) const
{
    /*   vec4 dir =	vec4(world, 1) * view_mat;
       vec3 res(dir / dir.w);
       //	res -= const_buffer.data().current.position;
       res.normalize();*/
    vec3 res = world - position;
    res.normalize();
    return res;
}

vec2 camera::to_local(vec3 world) const
{
    vec4 dir =	vec4(world, 1) * get_view_proj();
    vec2 res(dir.xy/ dir.w);
    res = res * 0.5 + vec2(0.5, 0.5);
    return res;
}

void camera::operator=(const camera& c)
{
    position = c.position;
    target = c.target;
    angle = c.angle;
    ratio = c.ratio;
    z_near = c.z_near;
    z_far = c.z_far;
    proj_mat = c.proj_mat;
    view_mat = c.view_mat;
    vangle = c.vangle;
}

const mat4x4& camera::get_proj() const
{
    return proj_mat;
}

const mat4x4& camera::get_view_proj() const
{
    return view_proj_mat;
}

const mat4x4& camera::get_view() const
{
    return view_mat;
}

const mat4x4& camera::get_inv_view_proj() const
{
    return inv_view_proj_mat;
}

void camera::set_projection_params(float z_near, float z_far)
{
    this->z_near = z_near;
    this->z_far = z_far;
    proj_mat.perspective(angle, ratio, z_near, z_far);
    fov_type = NORMAL;
}

void camera::set_projection_params(float angle, float ratio, float z_near, float z_far)
{
    this->angle = angle;
    this->ratio = ratio;
    this->z_near = z_near;
    this->z_far = z_far;
    this->vangle = angle / ratio;

    proj_mat.perspective(angle, ratio, z_near, z_far);
    fov_type = NORMAL;
}

void camera::set_projection_params(float l, float r, float t, float b, float z_near, float z_far)
{
    proj_mat.orthographic(l, r, t, b, z_near, z_far);
    fov_type = NORMAL;
}

camera::frustrum_points camera::get_points(float znear, float zfar)
{
    mat4x4 proj, v;
    if(fov_type==FOV_TYPE::NORMAL)
        proj.perspective(angle, ratio, znear, zfar);

#ifdef OCULUS_SUPPORT
    if (fov_type == FOV_TYPE::OCULUS)
    {
        ovrMatrix4f p = ovrMatrix4f_Projection(vr_fov, znear, zfar, ovrProjection_LeftHanded);

        proj = mat4x4{ p.M[0][0], p.M[1][0], p.M[2][0], p.M[3][0],
            p.M[0][1], p.M[1][1], p.M[2][1], p.M[3][1],
            p.M[0][2], p.M[1][2], p.M[2][2], p.M[3][2],
            p.M[0][3], p.M[1][3], p.M[2][3], p.M[3][3] };
    }
#endif
    v = view_mat * proj;
    (v).inverse();
    vec3 local_points[] =
    {
        { -1, -1, 0 },	//0
        { 1, -1, 0 },	//1
        { -1, 1, 0 },	//2
        { 1, 1, 0 },	//3

        { -1, -1, 1 },	//4
        { 1, -1, 1 },	//5
        { -1, 1, 1 },	//6
        { 1, 1, 1 }		//7
    };
    frustrum_points res;

    for (unsigned int i = 0; i < 8; i++)
    {
        vec4 t = vec4(local_points[i], 1) * v;
        res.p[i] = vec3(t.xyz) / t.w;
    }

    return res;
}

camera::frustrum_points camera::get_points(vec3 min, vec3 max)
{
    frustrum_points res;
    res.p[0] = {min.x,min.y,min.z};
    res.p[1] = { min.x,min.y,max.z };
    res.p[2] = { min.x,max.y,min.z };
    res.p[3] = { min.x,max.y,max.z };
    res.p[4] = { max.x,min.y,min.z };
    res.p[5] = { max.x,min.y,max.z };
    res.p[6] = { max.x,max.y,min.z };
    res.p[7] = { max.x,max.y,max.z };
    return res;
}

camera::camera()
{
    //  const_buffer.reset(new HAL::Buffer<shader_params>());
}

void first_person_camera::add_look(vec2 delta)
{
    // screen +x -> look right (yaw+), screen +y (down) -> look down (pitch-)
    angles.x += delta.x * look_sensitivity;
    angles.y -= delta.y * look_sensitivity;
    angles.y = Math::clamp(angles.y, -Math::m_pi_2 + Math::eps2, Math::m_pi_2 - Math::eps2);
}

void first_person_camera::frame_move(float dt)
{
    angles.y = Math::clamp(angles.y, -Math::m_pi_2 + Math::eps2, Math::m_pi_2 - Math::eps2);
    vec3 direction;
    direction.x = Math::sin(angles.x) * Math::cos(angles.y);
    direction.z = Math::cos(angles.x) *  Math::cos(angles.y);
    direction.y = Math::sin(angles.y);
    vec3 up(0, 1, 0);
    vec3 right = vec3::cross(up, direction).normalize();
    up = vec3::cross(direction, right).normalize();
    position += (fast_move ? 2000.0f : move_speed) * (
                    right * move_input.x +
                    direction * move_input.z +
                    up * move_input.y
                ) * dt;
    target = position + direction * 10;
    //     update();
}

void third_person_camera::input(float2 delta)
{
    angles += 2.0f * delta;
    angles.y = Math::clamp(angles.y, -Math::m_pi_2 + Math::eps2, Math::m_pi_2 - Math::eps2);
}

void third_person_camera::input(float value)
{
    zoom = Math::clamp(zoom*(1-value), 0.1f, 10.0f);
}

void third_person_camera::frame_move(float dt)
{
    vec3 direction;
    direction.x = Math::sin(angles.x) * Math::cos(angles.y);
    direction.z = Math::cos(angles.x) *  Math::cos(angles.y);
    direction.y = Math::sin(angles.y);
    vec3 up(0, 1, 0);
    vec3 right = vec3::cross(up, direction).normalize();
    up = vec3::cross(direction, right).normalize();
    position = direction*zoom;
    target = float3(0,0,0);
    //     update();
}

void camera::update(float2 offset)
{
	// `offset` is NDC-space jitter. Added into row 3 (a31/a32) rather than
	// post-multiplying a translation matrix — that's the row proportional to
	// view-space z (== clip.w), so it cancels uniformly across the w-divide
	// regardless of depth (a post-multiply translation would vary with depth).
	auto proj_jittered = proj_mat;
	proj_jittered.a31 += offset.x;
	proj_jittered.a32 += offset.y;

	vec3 _dir = target - position;// (0, 0, 1);
	vec3 _up (0, 1, 0);

	_dir = eye_rot.rotate(_dir);
	_up = eye_rot.rotate(_up);

	vec3 pos = position +this->offset;// (0, 1, 0);

	// Snapshot before view_mat is overwritten below — still last frame's pose,
	// giving last frame's unjittered view*proj for DLSS's clipToPrevClip.
	prev_view_proj_unjittered = view_mat * proj_mat;

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
	params.reprojectionProj = params.invViewProj*camera_cb.current.viewProj;

	mat4x4 test = view_proj_mat * inv_view_proj_mat;
	calculate(inv_view_proj_mat);
	vec3 right = vec3::cross(params.direction.xyz, _up);
	res_up = vec3::cross(params.direction.xyz, right);
	// UV-space jitter (half the NDC offset) — matches UniversalMaterial.hlsl's
	// motion-vector math so camera.jitter - prev_camera.jitter cancels correctly.
	params.jitter = { offset.x * 0.5f, offset.y * 0.5f, 0, 0 };
	

	for (int i = 0; i < 6; i++)
		params.frustum.planes[i] = Frustum::GetFrustumPlane(planes(i));


	camera_cb.prev = camera_cb.current;
	camera_cb.current = params;
}
