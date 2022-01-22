#pragma once
import Texture;
import Buffer;

struct visibility_update
{
	std::list<uint3> tiles_to_load;
	std::list<uint3> tiles_to_remove;

};
class VisibilityBuffer
{
	std::vector<unsigned char> clear_data;
	uint3 sizes;


protected:
	virtual void process_tile_readback(uint3 pos, char level) = 0;

public:
	Graphics::Texture::ptr buffer;

	Graphics::StructureBuffer<uint4>::ptr load_tiles_buffer;
	using ptr = std::shared_ptr<VisibilityBuffer>;

	VisibilityBuffer(uint3 sizes);

	std::future<visibility_update> waiter;

	//void wait_for_results();

	std::future<visibility_update> update(Graphics::CommandList::ptr& list);


};

class VisibilityBufferUniversal : public VisibilityBuffer
{
	virtual void process_tile_readback(uint3 pos, char level) override;

public:

	using VisibilityBuffer::VisibilityBuffer;
	using ptr = std::shared_ptr<VisibilityBufferUniversal>;

	std::function<void(uint3, int)> on_process;

};
