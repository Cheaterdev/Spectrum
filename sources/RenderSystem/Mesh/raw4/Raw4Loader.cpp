#include "pch_render.h"
#include "Raw4Loader.h"

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void Raw4SceneAdaptor::create_model(const Raw4Context &context, const raw4_block_model &model, const raw4_block_values* values)
{
	create_object(context, model, values);
}
//-----------------------------------------------------------------------------
void Raw4SceneAdaptor::create_body(const Raw4Context &context, const raw4_block_body &body, const raw4_block_values* values)
{
	create_object(context, body, values);
}
//-----------------------------------------------------------------------------
void Raw4SceneAdaptor::create_terrain(const Raw4Context &context, const raw4_block_terrain &terrain, const raw4_block_values* values)
{
	create_object(context, terrain, values);
}
//-----------------------------------------------------------------------------
void Raw4SceneAdaptor::create_billboard(const Raw4Context &context, const raw4_block_billboard &bboard, const raw4_block_values* values)
{
	create_object(context, bboard, values);
}
//-----------------------------------------------------------------------------
void Raw4SceneAdaptor::create_spline(const Raw4Context &context, const raw4_block_spline &spline, const raw4_block_values* values)
{
	create_object(context, spline, values);
}

//-----------------------------------------------------------------------------
template<typename T>
static inline const T* READ_BLOCK(const uint8_t* &p)
{
	T* block = (T*) p;
	p += sizeof(T);
	return block;
}

//-----------------------------------------------------------------------------
static inline void skip(const uint8_t* &p, uint32_t &size, uint32_t offset)
{
	p += offset;
	size -= offset;
}

//-----------------------------------------------------------------------------
template<typename T>
static T* read(const uint8_t* &p, uint32_t &size)
{
	T* obj = (T*) p;
	skip(p, size, sizeof(T));
	return obj;
}

//-----------------------------------------------------------------------------
template<typename T, typename SUBT>
static T* read(const uint8_t* &p, uint32_t &size, const uint32_t T::* subcount)
{
	T* obj = (T*) p;

	uint32_t local_size = sizeof(T) + obj->*subcount * sizeof(SUBT);

	skip(p, size, local_size);

	return obj;
}
//-----------------------------------------------------------------------------
void Raw4Loader::parse_block(Raw4Context &context, Raw4SceneAdaptor &adaptor, const uint8_t* &p, uint32_t global_size)
{
	adaptor.go_down();

	while(global_size > 0)
	{
		const raw4_block* block = READ_BLOCK<raw4_block>(p);
		uint32_t size = block->size - sizeof(raw4_block);

//		LOGI("raw4: block id %d, size %d, global_size %d", block->block_id, block->size, global_size);

		const raw4_block_object* obj;
		const raw4_block_model* model;
		const raw4_block_body* body;
		const raw4_block_terrain* terrain;
		const raw4_block_billboard* bboard;
		const raw4_block_spline* spline;

		const raw4_block_values* values;
		const raw4_block_control* control;

		const raw4_block_extension* ext;

		switch(block->block_id)
		{
		// context blocks, no childs
		case RAW4_BLOCK_STRINGS:
			context.strs = (raw4_block_strings*) block->data;
			p += size;
			break;

		case RAW4_BLOCK_TEXTURES:
			context.texs = (raw4_block_textures*) block->data;
			p += size;
			break;

		case RAW4_BLOCK_MATERIALS:
			context.mats = (raw4_block_materials*) block->data;
			p += size;
			break;

		case RAW4_BLOCK_BUFFERS:
			context.buffers = (raw4_block_buffers*) block->data;
			p += size;
			break;

		case RAW4_BLOCK_MESHS:
			context.meshs = (raw4_block_meshs*) block->data;
			p += size;
			break;

			// values, no childs
		case RAW4_BLOCK_VALUES:
			values = read<raw4_block_values, raw4_value>(p, size, &raw4_block_values::count);
			adaptor.add_values(context, *values);
			break;

			// controls, no childs
		case RAW4_BLOCK_CONTROL:
			control = (raw4_block_control*) block->data;
			adaptor.add_control(context, *control);
			p += size;
			break;

			// object blocks
		case RAW4_BLOCK_OBJECT:
			obj = read<raw4_block_object>(p, size);
			adaptor.create_object(context, *obj, get_values(context, p, size));
			if(size > 0) parse_block(context, adaptor, p, size);
			break;

		case RAW4_BLOCK_MODEL:
			model = read<raw4_block_model, raw4_model>(p, size, &raw4_block_model::lods_count);
			skip(p, size, model->additional_size);
			adaptor.create_model(context, *model, get_values(context, p, size));
			if(size > 0) parse_block(context, adaptor, p, size);
			break;

		case RAW4_BLOCK_BODY:
			body = read<raw4_block_body>(p, size);
			adaptor.create_body(context, *body, get_values(context, p, size));
			if(size > 0) parse_block(context, adaptor, p, size);
			break;

		case RAW4_BLOCK_TERRAIN:
			if(context.revision == 100) terrain = read<raw4_block_terrain, raw4_bvht_node_r100>(p, size, &raw4_block_terrain::nodes_count);
			else terrain = read<raw4_block_terrain, raw4_bvht_node>(p, size, &raw4_block_terrain::nodes_count);

			skip(p, size, terrain->sides_data_size);
			skip(p, size, terrain->sedges_count * sizeof(uint32_t));
			adaptor.create_terrain(context, *terrain, get_values(context, p, size));
			if(size > 0) parse_block(context, adaptor, p, size);
			break;

		case RAW4_BLOCK_BILLBOARD:
			bboard = read<raw4_block_billboard>(p, size);
			skip(p, size, bboard->objects_count*sizeof(raw4_block_billboard::bbobject));
			skip(p, size, bboard->indices_count*sizeof(uint16_t));
			adaptor.create_billboard(context, *bboard, get_values(context, p, size));
			if(size > 0) parse_block(context, adaptor, p, size);
			break;

		case RAW4_BLOCK_SPLINE:
			spline = read<raw4_block_spline>(p, size);
			skip(p, size, spline->count*spline->point_size);
			adaptor.create_spline(context, *spline, get_values(context, p, size));
			if(size > 0) parse_block(context, adaptor, p, size);
			break;

		case RAW4_BLOCK_EXTENSION:
			ext = read<raw4_block_extension>(p, size);
			skip(p, size, ext->size);
			adaptor.create_extension(context, *ext);
			if(size > 0) parse_block(context, adaptor, p, size);
			break;

		default:
		//	LOGW("raw4: unknown block id %d", block->block_id);
			p += size;
			break;
		}

		global_size -= block->size;
	}

	adaptor.go_up();
}

//-----------------------------------------------------------------------------
raw4_block_values* Raw4Loader::get_values(Raw4Context &context, const uint8_t* p, uint32_t size)
{
	if(size == 0) return nullptr;

	raw4_block* block = (raw4_block*)p;

	if(block->block_id == RAW4_BLOCK_VALUES) return (raw4_block_values*)block->data;
	else return nullptr;
}
//-----------------------------------------------------------------------------
bool Raw4Loader::Load(Assimp::IOSystem &io, std::string name , Raw4SceneAdaptor &adaptor)
{

	auto str = io.Open(name.c_str());

	std::vector<uint8_t> data(str->FileSize());

	str->Read(data.data(), data.size(), 1);

//	auto data = f->load_all();
	const uint8_t* p = reinterpret_cast<const uint8_t*>(data.data());// f->load_all();
	auto p_copy = p;
	// check magic number
	if(data.size() < sizeof(raw4_block_header)) return false;
	if(*((uint32_t*)p) != RAW4_MAGIC_NUMBER) return false;

	// read header
	const raw4_block_header* header = READ_BLOCK<raw4_block_header>(p);
//	LOGI("raw4: revision %d, data size %d bytes", header->revision, header->data_size);

	Raw4Context context;
	context.revision = header->revision;
	context.file_id = 0;// f->uid;
	context.base = p_copy;// f->data;

	// read blocks
	parse_block(context, adaptor, p, header->data_size);

	adaptor.finish(context);

	return true;
}
//-----------------------------------------------------------------------------
