#include "pch_render.h"
#include "Occluder.h"


//// BOOST_CLASS_EXPORT_IMPLEMENT(occluder)

bool occluder::is_inside(const Frustum& f)
{
 //   return true;

   if (!primitive) return true;

    return intersect(f, primitive_global.get());
}
