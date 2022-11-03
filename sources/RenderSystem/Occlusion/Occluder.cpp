module Graphics:Occluder;

import Math;

bool occluder::is_inside(const Frustum& f)
{
 //   return true;

   if (!primitive) return true;

    return intersect(f, primitive_global.get());
}
