module Graphics:Occluder;
import Core;

bool occluder::is_inside(const Frustum& f)
{
 //   return true;

   if (!primitive) return true;

    return intersect(f, primitive_global.get());
}

void occluder::set_primitive(Primitive::ptr primitive)
{
	this->primitive = primitive;
	if (primitive)
		primitive_global = primitive->clone();
}

void occluder::apply_transform(mat4x4& t)
{
	if (primitive_global && primitive)
	{
		primitive_global->apply_transform(primitive, t);

		min = primitive_global->get_min();
		max = primitive_global->get_max();
		if (primitive_childs)
		{
			min = vec3::min(primitive_childs->get_min(), min);
			max = vec3::max(primitive_childs->get_max(), max);
		}
	}
}

vec3 occluder::get_min()
{
	return min;
}

vec3 occluder::get_max()
{
	return max;
}

bool occluder::is_childs_inside(const Frustum& f)
{
	return false;
}
