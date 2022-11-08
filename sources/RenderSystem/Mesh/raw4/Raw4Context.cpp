
#include "Raw4Context.h"
#include <stdlib.h>
#include <string.h>
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
Raw4Context::Raw4Context()
{
	revision = 0;
	file_id = 0;
	base = nullptr;
	strs = nullptr;
	texs = nullptr;
	mats = nullptr;
	buffers = nullptr;
	meshs = nullptr;
}
//-----------------------------------------------------------------------------
Raw4Context::~Raw4Context()
{

}
//-----------------------------------------------------------------------------
const char* Raw4Context::GetString(uint32_t id) const
{
	if(strs == nullptr || id == 0 || id == RAW4_ID_EMPTY || id >= strs->count) return nullptr;
	else return (const char*)strs + strs->strings[id];
}
//-----------------------------------------------------------------------------
raw4_texture_data* Raw4Context::GetTexture(uint32_t id) const
{
	if(texs == nullptr || id == RAW4_ID_EMPTY || id >= texs->count) return nullptr;
	else return texs->data + id;
}
//-----------------------------------------------------------------------------
raw4_material_data* Raw4Context::GetMaterial(uint32_t id) const
{
	if(mats == nullptr || id == RAW4_ID_EMPTY || id >= mats->count) return nullptr;
	else return mats->materials + id;
}
//-----------------------------------------------------------------------------
raw4_buffer* Raw4Context::GetBuffer(uint32_t id) const
{
	if(buffers == nullptr || id == RAW4_ID_EMPTY || id >= buffers->count) return nullptr;
	else return buffers->buffers + id;
}
//-----------------------------------------------------------------------------
raw4_mesh_data* Raw4Context::GetMesh(uint32_t id) const
{
	if(meshs == nullptr || id == RAW4_ID_EMPTY || id >= meshs->count) return nullptr;
	else return meshs->meshs + id;
}
//-----------------------------------------------------------------------------
const raw4_value* Raw4Context::GetValue(const raw4_block_values* values, const char* vname) const
{
	if(values == nullptr) return nullptr;

	for(uint32_t i = 0; i < values->count; i++)
	{
		const char* name = GetString(values->values[i].name);

		if(strcmp(name, vname) == 0) return values->values + i;
	}

	return nullptr;
}
//-----------------------------------------------------------------------------
