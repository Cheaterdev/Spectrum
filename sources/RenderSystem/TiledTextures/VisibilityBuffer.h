#pragma once


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
	Render::Texture::ptr buffer;

	Render::StructuredBuffer<uint4>::ptr load_tiles_buffer;
	using ptr = std::shared_ptr<VisibilityBuffer>;

	VisibilityBuffer(ivec3 sizes);

	std::future<visibility_update> waiter;

	//void wait_for_results();

	std::future<visibility_update> update(Render::CommandList::ptr& list);


};

class VisibilityBufferUniversal : public VisibilityBuffer
{
	virtual void process_tile_readback(ivec3 pos, char level) override;

public:

	using VisibilityBuffer::VisibilityBuffer;
	using ptr = std::shared_ptr<VisibilityBufferUniversal>;

	std::function<void(ivec3, int)> on_process;

};
