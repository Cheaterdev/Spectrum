#ifndef _RAW4_HISTORY_H_
#define _RAW4_HISTORY_H_
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
union raw4_attribute_r101
{
	struct
	{
		uint8_t type;				// RAW4_ATTR_TYPE
		uint8_t count;				// count of components
		uint8_t format;				// format
		uint8_t normalized;			// true if need to be normalized
	};
	uint32_t value;
};

//------------------------------------------------------------------------------
struct raw4_bvht_node_r100
{
	raw4_aabb aabb;							// geometry bounding box
	uint32_t edgeL;						// left edge
	uint32_t edgeR;						// right edge
	uint32_t edgeD;						// down edge
	uint32_t edgeU;						// up edge
	uint32_t last;						// stop-flag
	vec2 uvoffset;						// min uv-coordinates
	float build_error;					// optimization error
	struct  
	{
		uint32_t offset;				// indices offset
		uint32_t count;					// indices count
	} variants[16];						// variants of geometry
	struct
	{
		uint32_t offset;				// offset in edges array
		uint32_t count;					// count of edges
		uint32_t offset_data;			// offset to data in global array
	} sides[4];							// sides mask
};

//------------------------------------------------------------------------------
#endif
