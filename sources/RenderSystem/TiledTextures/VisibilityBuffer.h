#pragma once
#include "Math/Vectors.h"
#include "DX12/Texture.h"
#include "DX12/Buffer.h"
#include "DX12/CommandList.h"


struct visibility_update
{
	std::list<ivec3> tiles_to_load;
	std::list<ivec3> tiles_to_remove;

};
class VisibilityBuffer
{
	std::vector<unsigned char> clear_data;
	ivec3 sizes;


protected:
	virtual void process_tile_readback(ivec3 pos, char level) = 0;

public:
	DX12::Texture::ptr buffer;

	DX12::StructuredBuffer<uint4>::ptr load_tiles_buffer;
	using ptr = std::shared_ptr<VisibilityBuffer>;

	VisibilityBuffer(ivec3 sizes);

	std::future<visibility_update> waiter;

	//void wait_for_results();

	std::future<visibility_update> update(DX12::CommandList::ptr& list);


};

class VisibilityBufferUniversal : public VisibilityBuffer
{
	virtual void process_tile_readback(ivec3 pos, char level) override;

public:

	using VisibilityBuffer::VisibilityBuffer;
	using ptr = std::shared_ptr<VisibilityBufferUniversal>;

	std::function<void(ivec3, int)> on_process;

};
