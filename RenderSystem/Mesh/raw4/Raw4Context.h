#ifndef _RAW4_CONTEXT_H_
#define _RAW4_CONTEXT_H_
//-----------------------------------------------------------------------------

#include "raw4.h"

//-----------------------------------------------------------------------------
struct Raw4Context
{
	uint32_t revision;
	uint32_t file_id;
	const uint8_t* base;
	raw4_block_strings* strs;
	raw4_block_textures* texs;
	raw4_block_materials* mats;
	raw4_block_buffers* buffers;
	raw4_block_meshs* meshs;

	Raw4Context();
	~Raw4Context();

	const char* GetString(uint32_t id) const;
	raw4_texture_data* GetTexture(uint32_t id) const;
	raw4_material_data* GetMaterial(uint32_t id) const;
	raw4_buffer* GetBuffer(uint32_t id) const;
	raw4_mesh_data* GetMesh(uint32_t id) const;
	const raw4_value* GetValue(const raw4_block_values* values, const char* vname) const;
};

//-----------------------------------------------------------------------------
#endif
