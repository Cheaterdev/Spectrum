module;
// aiMaterial::Get<T> forwards to static-inline helpers from material.h; they have to be visible where it is instantiated.
#include <assimp/material.h>
module assimp;

aiReturn ai_get(const aiMaterial& material, const AiMatKey& key, float& out)
{
	return material.Get(key.key, key.type, key.index, out);
}

aiReturn ai_get(const aiMaterial& material, const AiMatKey& key, aiColor3D& out)
{
	return material.Get(key.key, key.type, key.index, out);
}
