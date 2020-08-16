#pragma once

class VisibilityBuffer
{
	std::vector<unsigned char> clear_data;
	ivec3 sizes;
	
	std::future<bool> waiter;
protected:
	virtual void process_tile_readback(ivec3 pos, char level) = 0;

public:
	Render::ByteBuffer::ptr buffer;
	using ptr = std::shared_ptr<VisibilityBuffer>;

	VisibilityBuffer(ivec3 sizes);


	std::function<void(Render::Handle&)> uav();

	void wait_for_results();

	void update(Render::CommandList::ptr& list);


};

class VisibilityBufferUniversal : public VisibilityBuffer
{
	virtual void process_tile_readback(ivec3 pos, char level) override;

public:

	using VisibilityBuffer::VisibilityBuffer;
	using ptr = std::shared_ptr<VisibilityBufferUniversal>;

	std::function<void(ivec3, int)> on_process;

};
