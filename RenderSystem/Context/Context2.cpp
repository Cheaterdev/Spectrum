#include "pch.h"



void MeshRenderContext::use_material(int i, ptr& t)
{
    if (override_material)override_material->set(MESH_TYPE::ALL,t);
    else

        //  else
        //     materials[i]->set(t);
        if (mat_provider)
            mat_provider->use_material(i, t);
}