#ifndef _RAW4_H_
#define _RAW4_H_
//------------------------------------------------------------------------------

// Required vec3
//#include "stdtypes.h"

// Required AABB
//#include "gtypes.h"

// Required uint32_t, uint16_t, uint8_t types.
#include <stdint.h>

#pragma warning(disable:4200)
/*******************************************************************************

Little-endian format used.
All blocks aligned by 4 bytes.

Raw4 file structure:
	┌─────────┬─────────────────────────┐
	│ 4 bytes | MAGIC NUMBER            │
	├─────────┼─────────────────────────┤
	│ 4 bytes | format revision         │
	├─────────┼─────────────────────────┤
	│ 4 bytes | N = size of data blocks │
	├─────────┼─────────────────────────┤
	│         |        BLOCK 0          │
	|         ├─────────────────────────┤
	│ N bytes |        BLOCK 1          │
	|         ├─────────────────────────┤
	│         |         ....            │
	└─────────┴─────────────────────────┘

Block structure:
	┌─────────┬──────────────────────┐
	│ 4 bytes | block id             │
	├─────────┼──────────────────────┤
	│ 4 bytes | N+8 = size of block  │
	├─────────┼──────────────────────┤
	│ N bytes |        DATA          │
	└─────────┴──────────────────────┘

*******************************************************************************/

//------------------------------------------------------------------------------
#define RAW4_MAGIC_NUMBER			0x34574152	// "RAW4" symbols
#define RAW4_REVISION				102

//------------------------------------------------------------------------------
#define RAW4_BLOCK_STRINGS				0
#define RAW4_BLOCK_TEXTURES				1
#define RAW4_BLOCK_MATERIALS			2
#define RAW4_BLOCK_BUFFERS				3
#define RAW4_BLOCK_MESHS				4
#define RAW4_BLOCK_VALUES				5
#define RAW4_BLOCK_OBJECT				6
#define RAW4_BLOCK_MODEL				7
#define RAW4_BLOCK_BODY					8
#define RAW4_BLOCK_TERRAIN				9
#define RAW4_BLOCK_CONTROL				10
#define RAW4_BLOCK_BILLBOARD			11
#define RAW4_BLOCK_SPLINE				12
#define RAW4_BLOCK_MODEL_SUBMODELS		13
#define RAW4_BLOCK_EXTENSION			14

//------------------------------------------------------------------------------
#define RAW4_ID_EMPTY				0xFFFFFFFF

//------------------------------------------------------------------------------
#define RAW4_RENDER_POINTS			0	// points
#define RAW4_RENDER_LINES			1	// lines
#define RAW4_RENDER_LINE_STRIP		2	// line strip
#define RAW4_RENDER_TRIANGLES		3	// triangles
#define RAW4_RENDER_TRIANGLE_STRIP	4	// triangle strip

//------------------------------------------------------------------------------
#define RAW4_TEXTURE_TYPE_2D		0	// 2d texture
#define RAW4_TEXTURE_TYPE_3D		1	// 3d texture
#define RAW4_TEXTURE_TYPE_CUBE		2	// cube texture

//------------------------------------------------------------------------------
#define RAW4_TEXTURE_FILTER_NEAREST		0	// nearest filter
#define RAW4_TEXTURE_FILTER_LINEAR		1	// linear filter

//------------------------------------------------------------------------------
#define RAW4_TEXTURE_WRAP_CLAMP				0	// clamp to edge
#define RAW4_TEXTURE_WRAP_REPEAT			1	// repeat
#define RAW4_TEXTURE_WRAP_MIRRORED_REPEAT	2	// mirrored repeat

//------------------------------------------------------------------------------
#define RAW4_ATTR_TYPE_ALIGN		0		// for align data
#define RAW4_ATTR_TYPE_POS			1		// position {x,y,z}
#define RAW4_ATTR_TYPE_TANGENT		2		// tangent vector {x,y,z}
#define RAW4_ATTR_TYPE_BINORM		3		// binormal vector {x,y,z}
#define RAW4_ATTR_TYPE_NORMAL		4		// normal vector {x,y,z}
#define RAW4_ATTR_TYPE_UV			5		// texture coordinates {u,v} or {u,v,w}
#define RAW4_ATTR_TYPE_COLOR		6		// diffuse color {r,g,b,a}
#define RAW4_ATTR_TYPE_QUAT			7		// quaternion for transformation to tangent-space {x,y,z,w}
#define RAW4_ATTR_TYPE_WEIGHT		8		// weights

//------------------------------------------------------------------------------
const int RAW4_FORMAT_SIZE[] = {1, 1, 2, 2, 4, 4, 8, 8, 2, 4, 8, 10, 4};
#define RAW4_FORMAT_S8		0		// signed integer 8bit
#define RAW4_FORMAT_U8		1		// unsigned integer 8bit
#define RAW4_FORMAT_S16		2		// signed integer 16bit
#define RAW4_FORMAT_U16		3		// unsigned integer 16bit
#define RAW4_FORMAT_S32		4		// signed integer 32bit
#define RAW4_FORMAT_U32		5		// unsigned integer 32bit
#define RAW4_FORMAT_S64		6		// signed integer 64bit
#define RAW4_FORMAT_U64		7		// unsigned integer 64bit
#define RAW4_FORMAT_F16		8		// half precision float
#define RAW4_FORMAT_F32		9		// single precision float
#define RAW4_FORMAT_F64		10		// double precision float
#define RAW4_FORMAT_F80		11		// extended precision float
#define RAW4_FORMAT_X32		12		// fixed

//------------------------------------------------------------------------------
#define RAW4_RENDER_OPTS_CULL_FACE		(1)			// enable backface cull
#define RAW4_RENDER_OPTS_FRONT_CW		(1 << 1)	// CW mode if set, else CCW
#define RAW4_RENDER_OPTS_ZWRITE			(1 << 2)	// enable z-write
#define RAW4_RENDER_OPTS_ZTEST			(1 << 3)	// enable z-test
#define RAW4_RENDER_OPTS_ZFUNC_L		(1 << 4)	// z-func: less flag
#define RAW4_RENDER_OPTS_ZFUNC_G		(1 << 5)	// z-func: greater flag
#define RAW4_RENDER_OPTS_ZFUNC_E		(1 << 6)	// z-func: equal flag

//------------------------------------------------------------------------------
#define RAW4_ANIMATOR_NONE		0		// no animator
#define RAW4_ANIMATOR_SKIN		1		// skin animator
#define RAW4_ANIMATOR_MORPH		2		// morph animator (not supported yet)

//------------------------------------------------------------------------------
#define RAW4_BODY_MESH			0		// mesh
#define RAW4_BODY_BOX			1		// box
#define RAW4_BODY_SPHERE		2		// sphere
#define RAW4_BODY_CYLINDER		3		// cylinder
#define RAW4_BODY_CONE			4		// cone
#define RAW4_BODY_CAPSULE		5		// capsule
#define RAW4_BODY_LINK			6		// link

//------------------------------------------------------------------------------
#define RAW4_LOCK_PX			(1)			// position x lock
#define RAW4_LOCK_PY			(1<<1)		// position y lock
#define RAW4_LOCK_PZ			(1<<2)		// position z lock
#define RAW4_LOCK_RX			(1<<3)		// rotation x lock
#define RAW4_LOCK_RY			(1<<4)		// rotation y lock
#define RAW4_LOCK_RZ			(1<<5)		// rotation z lock
#define RAW4_LOCK_S				(1<<6)		// scale lock
#define RAW4_INHERIT_PX			(1<<7)		// inherit move x
#define RAW4_INHERIT_PY			(1<<8)		// inherit move y
#define RAW4_INHERIT_PZ			(1<<9)		// inherit move z
#define RAW4_INHERIT_RX			(1<<10)		// inherit rotation x
#define RAW4_INHERIT_RY			(1<<11)		// inherit rotation y
#define RAW4_INHERIT_RZ			(1<<12)		// inherit rotation z
#define RAW4_INHERIT_S			(1<<13)		// inherit scale
#define RAW4_HIDE				(1<<14)		// visible = false
#define RAW4_NOT_IMPORTANT		(1<<15)		// not important for rendering
#define RAW4_NOT_RENDERABLE		(1<<28)		// not renderable
#define RAW4_FLAG_CUSTOM1		(1<<29)		// custom flag 1
#define RAW4_FLAG_CUSTOM2		(1<<30)		// custom flag 2
#define RAW4_FLAG_CUSTOM3		(1<<31)		// custom flag 3

//------------------------------------------------------------------------------
#define RAW4_CONTROL_POSITION_XYZ	0			// xyz position control
#define RAW4_CONTROL_EULER			1			// euler rotation control
#define RAW4_CONTROL_LIMIT			2			// float limit control
#define RAW4_CONTROL_BEZIER			3			// bezier float control

//------------------------------------------------------------------------------
#define RAW4_BEZKEY_LINEAR		0			// linear interpolation
#define RAW4_BEZKEY_STEP		1			// step interpolation
#define RAW4_BEZKEY_TAN			2			// cubic bezier interpolation

//------------------------------------------------------------------------------
#define RAW4_ORT_CONST			0			// const boundary value
#define RAW4_ORT_CYCLE			1			// repeat animation

//------------------------------------------------------------------------------
#define RAW4_SPLINE_CLOSED		1			// spline is closed

struct raw4_aabb
{
	vec3 min;
	vec3 max;
};
//------------------------------------------------------------------------------
struct raw4_block_header
{
	uint32_t magic_number;			// must be 0x34574152 ("RAW4" string)
	uint32_t revision;				// format revision
	uint32_t data_size;				// total size of all blocks data
};

//------------------------------------------------------------------------------
struct raw4_block
{
	uint32_t block_id;				// block id
	uint32_t size;					// size of block
	uint8_t data[0];				// data
};

//------------------------------------------------------------------------------
struct raw4_block_strings
{
	uint32_t count;					// count of constant strings
	uint32_t strings[0];			// offsets array, first item always null
};

//------------------------------------------------------------------------------
struct raw4_texture_data
{
	uint32_t type;						// RAW4_TEXTURE_TYPE
	uint32_t name;						// string id
	uint8_t filter;						// RAW4_TEXTURE_FILTER
	uint8_t wrap_u;						// u-wrap mode
	uint8_t wrap_v;						// v-wrap mode
	uint8_t wrap_w;						// w-wrap mode
};

//------------------------------------------------------------------------------
struct raw4_block_textures
{
	uint32_t count;					// count of textures
	raw4_texture_data data[0];		// textures array
};

//------------------------------------------------------------------------------
struct raw4_material_data
{
	uint32_t name;							// string id
	uint32_t diffuse_color;					// diffuse color, hex RGBA
	uint32_t specular_color;				// specular color, hex RGBA
	uint32_t self_illumination_color;		// self-illumination color, hex RGBA
	uint32_t diffuse_texture;				// id of diffuse texture
	uint32_t specular_texture;				// id of specular texture
	uint32_t bump_texture;					// id of bump texture
	float specular_light_level;				// specular light: level
	float specular_light_glossiness;		// specular light: glossiness
};

//------------------------------------------------------------------------------
struct raw4_block_materials
{
	uint32_t count;						// count of materials
	raw4_material_data materials[0];	// materials array
};

//------------------------------------------------------------------------------
struct raw4_buffer
{
	uint32_t size;						// size data in bytes
	uint32_t offset;					// offset to data
};

//------------------------------------------------------------------------------
struct raw4_block_buffers
{
	uint32_t count;						// count of buffers
	raw4_buffer buffers[0];				// array of buffers
};

//------------------------------------------------------------------------------
struct raw4_attribute
{
	uint8_t type;				// RAW4_ATTR_TYPE
	uint8_t count;				// count of components
	uint8_t format;				// format
	uint8_t normalized;			// true if need to be normalized
	uint32_t stride;			// stride
	uint32_t offset;			// offset
};

//------------------------------------------------------------------------------
struct raw4_vertex_array
{
	uint32_t vertices;					// vertex buffer id
	uint32_t attributes_count;			// count attributes in vertex
	raw4_attribute attrs[0];			// array of attributes
};

//------------------------------------------------------------------------------
struct raw4_mesh_data
{
	raw4_aabb aabb;							// aabb for this mesh
	vec3 scale;							// scale for normalized position
	vec3 move;							// move for normalized position

	uint32_t render_type;				// render type (RAW4_RENDER)

	uint32_t index_format;				// must be u8, u16 or u32
	uint32_t index_count;				// index count
	uint32_t indices;					// indices buffer id

	uint32_t arrays_count;				// vertex arrays count
	uint32_t arrays_offset;				// offset to arrays
};

//------------------------------------------------------------------------------
struct raw4_block_meshs
{
	uint32_t count;						// count of meshs
	raw4_mesh_data meshs[0];			// meshs array
};

//------------------------------------------------------------------------------
struct raw4_value
{
	uint32_t name;					// name of value
	uint32_t ival;					// integer value
	float fval;						// float value
	uint32_t sval;					// string value
};

//------------------------------------------------------------------------------
struct raw4_block_values
{
	uint32_t count;					// count of values
	raw4_value values[0];			// values array
};

//------------------------------------------------------------------------------
struct raw4_control
{
	uint32_t type;					// RAW4_CONTROL type
};

//------------------------------------------------------------------------------
struct raw4_triple_control : public raw4_control
{
	uint32_t x;				// offset to x-control
	uint32_t y;				// offset to y-control
	uint32_t z;				// offset to z-control
};

//------------------------------------------------------------------------------
typedef raw4_triple_control raw4_position_xyz_control;
typedef raw4_triple_control raw4_euler_control;

//------------------------------------------------------------------------------
struct raw4_limit : public raw4_control
{
	vec2 interval;					// interval for value
};

//------------------------------------------------------------------------------
struct raw4_bezier_key
{
	struct { uint16_t in, out; } type;	// RAW4_BEZKEY type
	float time;							// time in sec
	float value;						// value
	float intan;						// in tangent
	float inlen;						// in length
	float outtan;						// out tangent
	float outlen;						// out length
};

//------------------------------------------------------------------------------
struct raw4_bezier : public raw4_control
{
	struct { uint16_t before, after; } ort;		// out-of-range type (RAW4_ORT)
	uint32_t keys_count;						// keys count
	raw4_bezier_key keys[0];					// array of keys
};

//------------------------------------------------------------------------------
struct raw4_block_control
{
	uint32_t position;		// offset to position control
	uint32_t rotation;		// offset to rotation control
	uint32_t scale;			// offset to scale control
};

//------------------------------------------------------------------------------
struct raw4_block_object
{
	uint32_t local_id;				// local id
	uint32_t name;					// name of object
	uint32_t rot_order;				// rotation order
	vec3 position;					// {x,y,z} position
	vec3 rotation;					// {x,y,z} rotation by axis
	float scale;					// uniform scale
	vec3 offset_pos;				// offset position
	vec3 offset_rot;				// offset rotation
	uint32_t flags;					// locks, inheritance and customs
	mat4x3 lparent;					// local parent matrix 4x3
};

//------------------------------------------------------------------------------
struct raw4_model_animator
{
	uint32_t type;					// type of animator
	uint32_t offset;				// offset to animator data
};

//------------------------------------------------------------------------------
struct raw4_model
{
	float threshold;				// view threshold
	uint32_t mesh;					// mesh id
	uint32_t material;				// material id
	raw4_model_animator animator;	// animator
};

//------------------------------------------------------------------------------
struct raw4_block_model : public raw4_block_object
{
	uint32_t render_opts;			// render options
	uint32_t additional_size;		// additional animator data size
	uint32_t lods_count;			// lods count
	raw4_model lods[0];				// lods
};

//------------------------------------------------------------------------------
struct raw4_bone
{
	uint32_t local_id;					// local id of bone
	mat4x4 diff;							// difference transform
};

//------------------------------------------------------------------------------
struct raw4_skin_animator
{
	uint32_t bones_count;				// bones count
	raw4_bone bones[0];					// bones array
};

//------------------------------------------------------------------------------
struct raw4_body_mesh_data
{
	uint32_t mesh;					// mesh id
};

//------------------------------------------------------------------------------
struct raw4_body_box_data
{
	float width;					// size by axis X (-width/2, +width/2)
	float length;					// size by axis Y (-length/2, +length/2)
	float height;					// size by axis Z (0, height)
};

//------------------------------------------------------------------------------
struct raw4_body_sphere_data
{
	float radius;					// radius of sphere
};

//------------------------------------------------------------------------------
struct raw4_body_ccc_data			// cylinder, cone, capsule data
{
	float radius;					// radius
	float height;					// size by axis Z (0, height)
};

//------------------------------------------------------------------------------
struct raw4_body_link				// link
{
	uint32_t id0;					// first object id
	uint32_t id1;					// second object id
};

//------------------------------------------------------------------------------
struct raw4_block_body : public raw4_block_object
{
	uint32_t type;						// RAW4_BODY
	union
	{
		raw4_body_mesh_data mesh;		// mesh
		raw4_body_box_data box;			// box
		raw4_body_sphere_data sphere;	// sphere
		raw4_body_ccc_data ccc;			// cylinder, cone, capsule
		raw4_body_link link;			// link
	};
};

//------------------------------------------------------------------------------
struct raw4_bvht_node
{
	raw4_aabb aabb;							// geometry bounding box
	uint32_t edgeL;						// left edge
	uint32_t edgeR;						// right edge
	uint32_t edgeD;						// down edge
	uint32_t edgeU;						// up edge
	uint32_t last;						// stop-flag
	vec2 uvoffset;						// min uv-coordinates
	float build_error;					// optimization error
	uint32_t voffset;					// vertex offset
	struct  
	{
		uint32_t offset;				// indices offset
		uint32_t count;					// indices count + strip flag(31th bit)
	} variants[16];						// variants of geometry
	struct
	{
		uint32_t offset;				// offset in edges array
		uint32_t count;					// count of edges
		uint32_t offset_data;			// offset to data in global array
	} sides[4];							// sides mask
};

//------------------------------------------------------------------------------
struct raw4_block_terrain : public raw4_block_object
{
	raw4_aabb aabb;							// terrain bounding box
	uint32_t texture_r;					// texture R file
	uint32_t texture_g;					// texture G file
	uint32_t texture_b;					// texture B file
	uint32_t texture_0;					// texture 0 file
	uint32_t texture_selector;			// selector texture file
	uint32_t texture_adds;				// additive/shadow texture file 
	uint32_t texture_diffuse;			// low-res diffuse texture file
	uint32_t heightmap;					// heightmap

	uint32_t vbuffer_id;				// vertex buffer
	uint32_t ibuffer_id;				// total indices buffer
	uint32_t index_format;				// index format

	uint32_t sedges_count;				// count of separated edges
	uint32_t sides_data_size;			// size of global array of sides data
	uint32_t nodes_count;				// nodes count
	raw4_bvht_node nodes[0];			// array of nodes
};

//------------------------------------------------------------------------------
struct raw4_block_billboard : public raw4_block_object
{
	vec4 bsphere;						// bounding sphere
	uint32_t texture;					// texture
	uint32_t vbuffer_id;				// vertex buffer
	uint32_t voffset;					// offset in vertex buffer
	uint32_t objects_count;				// bb-objects count
	uint32_t indices_count;				// indices count
	struct bbobject{
		vec3 center;					// center bb-object
		uint32_t weights;				// weights
		uint32_t offset;				// indices offset
		uint32_t count;					// indices count
	}objects[0];						// bb-objects array

	// ... indices data
};

//------------------------------------------------------------------------------
struct raw4_block_spline : public raw4_block_object
{
	uint32_t count;						// point count
	uint32_t flags;						// flags
	uint32_t point_size;				// point data size (in bytes)

	// ... data
};

//------------------------------------------------------------------------------
struct raw4_block_extension
{
	uint32_t id;						// extension id
	uint32_t size;						// data size

	// ... data
};

//------------------------------------------------------------------------------
#include "raw4_history.h"

#pragma warning(default:4200)
//------------------------------------------------------------------------------
#endif
