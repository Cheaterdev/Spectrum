export module Graphics:Camera;

import HAL;
import Core;

import "windows/windows.h";
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

            box get_bounds_in(mat4x4 view_proj) const;
        };
    private:
        mat4x4 proj_mat;
        mat4x4 view_mat;

        mat4x4 view_proj_mat;
        mat4x4 inv_view_proj_mat;

        // Last completed frame's UNJITTERED view*proj, for DLSS's clipToPrevClip.
        mat4x4 prev_view_proj_unjittered;
        //Frustum frustum;
    
        shader_params params;


    public:
		camera_const camera_cb;

        vec3 to_direction(vec2 local) const;

        vec3 to_direction(vec3 world) const;


        vec2 to_local(vec3 world) const;

        void operator=(const camera& c);

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
        const mat4x4& get_proj() const;
		const mat4x4& get_view_proj() const;

			const mat4x4& get_view() const;
        const mat4x4& get_inv_view_proj() const;

        // Only valid from the second update() call onward.
        const mat4x4& get_prev_view_proj_unjittered() const { return prev_view_proj_unjittered; }

		void update(float2 offset = { 0, 0 });

		void set_projection_params( float z_near, float z_far);

        void set_projection_params(float angle, float ratio, float z_near, float z_far);

        void set_projection_params(float l, float r, float t, float b, float z_near, float z_far);

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
    
        virtual frustrum_points get_points(float znear, float zfar);

		frustrum_points get_points(vec3 min, vec3 max);

        camera();

    private:

};
//TODO: move to game framework
export class first_person_camera : public camera
{
        vec2 angles;
    public:
        // Input state, driven by the owning GUI element (no global key polling).
        vec3  move_input       = {0, 0, 0}; // x=right(A/D), y=up(E/Q), z=forward(W/S)
        bool  fast_move        = false;
        float move_speed       = 20.0f;
        float look_sensitivity = 0.004f;

        void add_look(vec2 delta); // mouse-look: rotate by a screen-space drag delta
        void frame_move(float dt);
};


export class third_person_camera : public camera
{
    vec2 angles ={0,0};
    float zoom = 5;
    public:
        void input(float2 delta);

     void input(float value);

        void frame_move(float dt);
};