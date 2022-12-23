export module Graphics:Camera;

import HAL;
import Core;

import windows;
export class camera : public Frustum
{
    public:

        using shader_params = Table::Camera;

#pragma pack(push, 4)
        struct camera_const
        {
            shader_params current;
            shader_params prev;
        };
#pragma pack(pop)

        struct frustrum_points
        {
            vec3 p[8];

            box get_bounds_in(mat4x4 view_proj) const
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
        };
    private:
        mat4x4 proj_mat;
        mat4x4 view_mat;

        mat4x4 view_proj_mat;
        mat4x4 inv_view_proj_mat;
        //Frustum frustum;
    
        shader_params params;


    public:
		camera_const camera_cb;

        vec3 to_direction(vec2 local) const
        {
            vec4 dir(local * 2 - vec2(1, 1), 1, 1);
            dir.y = -dir.y;
            dir = dir * get_inv_view_proj();
            vec3 res(dir.xyz / dir.w);
            res -= camera_cb.current.position.xyz;
            res.normalize();
            return res;
        }

        vec3 to_direction(vec3 world) const
        {
            /*   vec4 dir =	vec4(world, 1) * view_mat;
               vec3 res(dir / dir.w);
               //	res -= const_buffer.data().current.position;
               res.normalize();*/
            vec3 res = world - position;
            res.normalize();
            return res;
        }


        vec2 to_local(vec3 world) const
        {
            vec4 dir =	vec4(world, 1) * get_view_proj();
            vec2 res(dir.xy/ dir.w);
            res = res * 0.5 + vec2(0.5, 0.5);
            return res;
        }
        void operator=(const camera& c)
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

		quat eye_rot;
        vec3 position;
        vec3 target;
        vec3 up = vec3(0, 1, 0);
		vec3 offset=vec3(0,0,0);
        vec3 res_up;
        float angle, ratio, z_near, z_far, vangle;

#ifdef OCULUS_SUPPORT
		ovrFovPort vr_fov;
#endif
		enum FOV_TYPE
		{
			NORMAL,
			OCULUS
		}fov_type= NORMAL;
        /*	operator Frustum()
        	{
        		return frustum;
        	}

        	operator const Frustum() const
        	{
        		return frustum;
        	}

        	*/
        const mat4x4& get_proj() const
        {
            return proj_mat;
        }
		const mat4x4& get_view_proj() const
		{
			return view_proj_mat;
		}

			const mat4x4& get_view() const
			{
				return view_mat;
			}
        const mat4x4& get_inv_view_proj() const
        {
            return inv_view_proj_mat;
        }

		void update(float2 offset = { 0, 0 });

		void set_projection_params( float z_near, float z_far)
		{
			this->z_near = z_near;
			this->z_far = z_far;
			proj_mat.perspective(angle, ratio, z_near, z_far);
			fov_type = NORMAL;
		}

        void set_projection_params(float angle, float ratio, float z_near, float z_far)
        {
            this->angle = angle;
            this->ratio = ratio;
            this->z_near = z_near;
            this->z_far = z_far;
            this->vangle = angle / ratio;

            proj_mat.perspective(angle, ratio, z_near, z_far);
			fov_type = NORMAL;
        }
        void set_projection_params(float l, float r, float t, float b, float z_near, float z_far)
        {
            proj_mat.orthographic(l, r, t, b, z_near, z_far);
			fov_type = NORMAL;
        }

#ifdef OCULUS_SUPPORT
		void set_projection_params(ovrFovPort vr_fov, float z_near, float z_far)
		{
			fov_type = OCULUS;
			this->vr_fov = vr_fov;
			ovrMatrix4f p = ovrMatrix4f_Projection(vr_fov, z_near, z_far, ovrProjection_LeftHanded);

			proj_mat = mat4x4{ p.M[0][0], p.M[1][0], p.M[2][0], p.M[3][0],
				p.M[0][1], p.M[1][1], p.M[2][1], p.M[3][1],
				p.M[0][2], p.M[1][2], p.M[2][2], p.M[3][2],
				p.M[0][3], p.M[1][3], p.M[2][3], p.M[3][3] };


		}
#endif
    
        virtual frustrum_points get_points(float znear, float zfar)
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

		frustrum_points get_points(vec3 min, vec3 max)
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
        camera()
        {
            //  const_buffer.reset(new HAL::Buffer<shader_params>());
        }

    private:

};
//TODO: move to game framework
export class first_person_camera : public camera
{
        vec2 angles;
    public:

        void frame_move(float dt)
        {
            angles += 2.0f * vec2(!!GetAsyncKeyState('L') - !!GetAsyncKeyState('J'), !!GetAsyncKeyState('I') - !!GetAsyncKeyState('K')) * dt;
            angles.y = Math::clamp(angles.y, -Math::m_pi_2 + Math::eps2, Math::m_pi_2 - Math::eps2);
            vec3 direction;
            direction.x = Math::sin(angles.x) * Math::cos(angles.y);
            direction.z = Math::cos(angles.x) *  Math::cos(angles.y);
            direction.y = Math::sin(angles.y);
            vec3 up(0, 1, 0);
            vec3 right = vec3::cross(up, direction).normalize();
            up = vec3::cross(direction, right).normalize();
            position += (!!GetAsyncKeyState(VK_LSHIFT) ? 2000.0f : 20.0f) * (
                            right * static_cast<float>(!!GetAsyncKeyState('D') - !!GetAsyncKeyState('A')) +
                            direction * static_cast<float>(!!GetAsyncKeyState('W') - !!GetAsyncKeyState('S')) +
                            up * static_cast<float>(!!GetAsyncKeyState('E') - !!GetAsyncKeyState('Q'))
                        ) * dt;
            target = position + direction * 10;
            //     update();
        }
};