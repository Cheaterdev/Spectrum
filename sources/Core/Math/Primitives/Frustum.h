#pragma once
#include "Plane.h"

class Frustum : public Primitive
{
        vec3 global_points[8];
        mat4x4 inverse_mat;
    public:
        std::shared_ptr<Primitive>clone() override;
        mat4x4 get_transform() const;

        vec3 get_max() override;
        vec3 get_min() override;
        const primitive_types get_type() const override;
        void normalize();
        void calculate(const mat4x4& inverse_mat);

        enum planes
        {
            plane_near,
            plane_far,
            plane_left,
            plane_right,
            plane_top,
            plane_bottom,

            COUNT
        };

        Plane& GetFrustumPlane(planes plane);
        const Plane& GetFrustumPlane(planes plane) const;
    protected:
        Plane p[planes::COUNT];

private:
    SERIALIZE()
    {
        ar& SAVE_PARENT(Primitive);
        ar& NVP(global_points)& NVP(inverse_mat)& NVP(p);
    }
};
