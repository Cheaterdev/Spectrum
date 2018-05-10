#ifndef _RAW4_LOADER_H_
#define _RAW4_LOADER_H_
//-----------------------------------------------------------------------------

#include "raw4.h"
//#include "FS.h"
#include "Raw4Context.h"
#include "../../../3rdparty/assimp.h"
//-----------------------------------------------------------------------------
struct Raw4SceneAdaptor
{
	virtual ~Raw4SceneAdaptor(){}

	virtual void go_down(){}
	virtual void go_up(){}

	virtual void create_object(const Raw4Context &context, const raw4_block_object &obj, const raw4_block_values* values)=0;

	virtual void create_model(const Raw4Context &context, const raw4_block_model &model, const raw4_block_values* values);
	virtual void create_body(const Raw4Context &context, const raw4_block_body &body, const raw4_block_values* values);
	virtual void create_terrain(const Raw4Context &context, const raw4_block_terrain &terrain, const raw4_block_values* values);
	virtual void create_billboard(const Raw4Context &context, const raw4_block_billboard &bboard, const raw4_block_values* values);
	virtual void create_spline(const Raw4Context &context, const raw4_block_spline &spline, const raw4_block_values* values);

	virtual void add_values(const Raw4Context &context, const raw4_block_values &values){}
	virtual void add_control(const Raw4Context &context, const raw4_block_control &control){}

	virtual void create_extension(const Raw4Context &context, const raw4_block_extension &ext){}

	virtual void finish(const Raw4Context &context){}
};

//-----------------------------------------------------------------------------
class Raw4Loader
{
private:
	static void parse_block(Raw4Context &context, Raw4SceneAdaptor &adaptor, const uint8_t* &p, uint32_t global_size);
	static raw4_block_values* get_values(Raw4Context &context, const uint8_t* p, uint32_t size);

public:
	static bool Load(Assimp::IOSystem &io, std::string name , Raw4SceneAdaptor &adaptor);
};
//-----------------------------------------------------------------------------
#endif
