module Core:Holdable;

import :Data;
import stl.core;

Holder Holder::clone()
{
	Holder result;
	for (const auto& [a, b] : objects)
	{
		result.objects[a] = b;
	}
	return result;
}

UniversalContext UniversalContext::clone()
{
	UniversalContext result;
	result.holder = holder.clone();
	return result;
}
