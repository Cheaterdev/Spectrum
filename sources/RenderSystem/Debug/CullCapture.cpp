module Graphics:CullCapture;
import RenderSystem;

import Core;
import HAL;

using namespace HAL;

CullCapture::CullCapture()
{
	stamps = std::make_shared<virtual_gpu_buffer<uint>>(RenderSystem::get().device(), max_meshes, counterType::NONE,
		HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
	stamps->buffer.resource->set_name("CullCapture_Stamps");
}

void CullCapture::next_frame()
{
	// 30 bits of frame id; 0 is reserved for "nothing captured yet", which a
	// zeroed stamp buffer must never match.
	frame = (frame + 1) & ((1u << 30) - 1);
	if (frame == 0)
		frame = 1;
}

void CullCapture::prepare(HAL::CommandList& list)
{
	stamps->reserve(list, max_meshes);
	if (!cleared)
	{
		list.get_compute().clear(stamps->buffer);
		cleared = true;
	}
}
