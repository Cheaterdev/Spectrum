#include "pch_render.h"
#include "GUI/Renderer/Renderer.h"

namespace GUI
{
	NinePatch::NinePatch()
	{

		std::vector<unsigned int> index_data(9 * 2 * 3);
		auto data = index_data.data();

		for (auto i = 0; i < 3; i++)
			for (auto j = 0; j < 3; j++)
			{
				(*data++) = i * 4 + j;
				(*data++) = i * 4 + j + 1;
				(*data++) = i * 4 + j + 4;
				(*data++) = i * 4 + j + 4;
				(*data++) = i * 4 + j + 1;
				(*data++) = i * 4 + j + 5;
			}

		index_buffer = Graphics::IndexBuffer::make_buffer(index_data);
	}

	void NinePatch::draw(Graphics::context& c, GUI::Texture& item, rect r, Graphics::PipelineState::ptr pipeline_state)
	{
		if (current_state && current_state != pipeline_state)
		{
			flush(c);

		}

		bool added = false;
		if (item.srv)
		{
			added = true;
			textures_handles.emplace_back(item.srv);

			//	c.command_list->use_resource(item.srv[0]->resource_info->resource_ptr);
		}
		else
			if (item.texture)
			{
				added = true;
				textures_handles.emplace_back(item.texture->texture_2d().texture2D);
				//	c.command_list->use_resource(item.texture.get());
			}

		if (!added && current_state == GetPSO<PSOS::NinePatch>())
		{
			return;
		}
		current_state = pipeline_state;

		vertexes.resize(vertexes.size() + 16);

		auto _vertexes = vertexes.data() + vertexes.size() - 16;

		auto margin = c.scale * item.margins;
		auto padding = c.scale * item.padding;
		r += rect{ -margin.left, -margin.top, margin.left + margin.right, margin.top + margin.bottom };

		float tl = 0, tt = 0, tr = 0, tb = 0;

		if (item.texture)
		{
			tl = static_cast<float>(item.padding.left) / item.texture->get_desc().as_texture().Dimensions.x;
			tt = static_cast<float>(item.padding.top) / item.texture->get_desc().as_texture().Dimensions.y;
			tr = static_cast<float>(item.padding.right) / item.texture->get_desc().as_texture().Dimensions.x;
			tb = static_cast<float>(item.padding.bottom) / item.texture->get_desc().as_texture().Dimensions.y;
		}

		sizer new_tc = item.tc;
		if (item.tiled)
		{
			new_tc.right = r.w / item.texture->get_size().x;
			new_tc.bottom = r.h / item.texture->get_size().y;

		}

		if (new_tc.right < new_tc.left) {
			tl = -tl;
			tr = -tr;
		}

		if (new_tc.bottom < new_tc.top) {
			tt = -tt;
			tb = -tb;
		}

		float scale = 1;
		sizer clip = c.ui_clipping;
		float4 x_pos = { 0, padding.left * scale, r.size.x - padding.right * scale, r.size.x };
		float4 y_pos = { 0, padding.top * scale, r.size.y - padding.bottom * scale, r.size.y };

		x_pos += float4(r.pos.x, r.pos.x, r.pos.x, r.pos.x) + float4(c.offset.x, c.offset.x, c.offset.x, c.offset.x);
		y_pos += float4(r.pos.y, r.pos.y, r.pos.y, r.pos.y) + float4(c.offset.y, c.offset.y, c.offset.y, c.offset.y);

		float4 x_tc = { new_tc.left, new_tc.left + tl, new_tc.right - tr, new_tc.right };
		float4 y_tc = { new_tc.top, new_tc.top + tt, new_tc.bottom - tb, new_tc.bottom };

		if (clip.left_top.x < x_pos.w && clip.left_top.x > x_pos.x) {
			float new_tc = x_tc.x;
			if (clip.left_top.x < x_pos.y) {
				float l = (clip.left_top.x - x_pos.x) / (x_pos.y - x_pos.x);
				new_tc = x_tc.x * (1 - l) + x_tc.y * (l);
				x_pos.x = clip.left_top.x;
				x_tc.x = new_tc;

			}
			else if (clip.left_top.x < x_pos.z) {
				float l = (clip.left_top.x - x_pos.y) / (x_pos.z - x_pos.y);
				new_tc = x_tc.y * (1 - l) + x_tc.z * (l);
				x_pos.x = x_pos.y = clip.left_top.x;
				x_tc.x = x_tc.y = new_tc;

			}
			else {
				float l = (clip.left_top.x - x_pos.z) / (x_pos.w - x_pos.z);
				new_tc = x_tc.z * (1 - l) + x_tc.w * (l);

				x_pos.x = x_pos.y = x_pos.z = clip.left_top.x;
				x_tc.z = x_tc.x = x_tc.y = new_tc;

			}
		}

		if (clip.right_bottom.x < x_pos.w && clip.right_bottom.x > x_pos.x) {
			float new_tc = x_tc.x;
			if (clip.right_bottom.x < x_pos.y) {
				float l = (clip.right_bottom.x - x_pos.x) / (x_pos.y - x_pos.x);
				new_tc = x_tc.x * (1 - l) + x_tc.y * (l);
				x_pos.y = x_pos.z = x_pos.w = clip.right_bottom.x;
				x_tc.y = x_tc.z = x_tc.w = new_tc;

			}
			else if (clip.right_bottom.x < x_pos.z) {
				float l = (clip.right_bottom.x - x_pos.y) / (x_pos.z - x_pos.y);
				new_tc = x_tc.y * (1 - l) + x_tc.z * (l);
				x_pos.z = x_pos.w = clip.right_bottom.x;
				x_tc.z = x_tc.w = new_tc;

			}
			else {
				float l = (clip.right_bottom.x - x_pos.z) / (x_pos.w - x_pos.z);
				new_tc = x_tc.z * (1 - l) + x_tc.w * (l);

				x_pos.w = clip.right_bottom.x;
				x_tc.w = new_tc;

			}
		}

		if (clip.left_top.y < y_pos.w && clip.left_top.y > y_pos.x) {
			float new_tc = y_tc.x;
			if (clip.left_top.y < y_pos.y) {
				float l = (clip.left_top.y - y_pos.x) / (y_pos.y - y_pos.x);
				new_tc = y_tc.x * (1 - l) + y_tc.y * (l);
				y_pos.x = clip.left_top.y;
				y_tc.x = new_tc;

			}
			else if (clip.left_top.y < y_pos.z) {
				float l = (clip.left_top.y - y_pos.y) / (y_pos.z - y_pos.y);
				new_tc = y_tc.y * (1 - l) + y_tc.z * (l);
				y_pos.x = y_pos.y = clip.left_top.y;
				y_tc.x = y_tc.y = new_tc;

			}
			else {
				float l = (clip.left_top.y - y_pos.z) / (y_pos.w - y_pos.z);
				new_tc = y_tc.z * (1 - l) + y_tc.w * (l);

				y_pos.x = y_pos.y = y_pos.z = clip.left_top.y;
				y_tc.z = y_tc.x = y_tc.y = new_tc;

			}
		}

		if (clip.right_bottom.y < y_pos.w && clip.right_bottom.y > y_pos.x) {
			float new_tc = y_tc.x;
			if (clip.right_bottom.y < y_pos.y) {
				float l = (clip.right_bottom.y - y_pos.x) / (y_pos.y - y_pos.x);
				new_tc = y_tc.x * (1 - l) + y_tc.y * (l);
				y_pos.y = y_pos.z = y_pos.w = clip.right_bottom.y;
				y_tc.y = y_tc.z = y_tc.w = new_tc;

			}
			else if (clip.right_bottom.y < y_pos.z) {
				float l = (clip.right_bottom.y - y_pos.y) / (y_pos.z - y_pos.y);
				new_tc = y_tc.y * (1 - l) + y_tc.z * (l);
				y_pos.z = y_pos.w = clip.right_bottom.y;
				y_tc.z = y_tc.w = new_tc;

			}
			else {
				float l = (clip.right_bottom.y - y_pos.z) / (y_pos.w - y_pos.z);
				new_tc = y_tc.z * (1 - l) + y_tc.w * (l);

				y_pos.w = clip.right_bottom.y;
				y_tc.w = new_tc;

			}
		}

		if (x_pos.x >= x_pos.w || y_pos.x >= y_pos.w) {
			//	LOGE("```` ARE Y RESIOUS?");
		}

		if (y_pos.y > y_pos.z) {
			y_pos.y = y_pos.z = (y_pos.y + y_pos.z) / 2;
		}

		if (x_pos.y > x_pos.z) {
			x_pos.y = x_pos.z = (x_pos.y + x_pos.z) / 2;
		}
		_vertexes[0].pos = float2(x_pos.x, y_pos.x);
		_vertexes[1].pos = float2(x_pos.y, y_pos.x);
		_vertexes[2].pos = float2(x_pos.z, y_pos.x);
		_vertexes[3].pos = float2(x_pos.w, y_pos.x);

		_vertexes[4].pos = float2(x_pos.x, y_pos.y);
		_vertexes[5].pos = float2(x_pos.y, y_pos.y);
		_vertexes[6].pos = float2(x_pos.z, y_pos.y);
		_vertexes[7].pos = float2(x_pos.w, y_pos.y);

		_vertexes[8].pos = float2(x_pos.x, y_pos.z);
		_vertexes[9].pos = float2(x_pos.y, y_pos.z);
		_vertexes[10].pos = float2(x_pos.z, y_pos.z);
		_vertexes[11].pos = float2(x_pos.w, y_pos.z);

		_vertexes[12].pos = float2(x_pos.x, y_pos.w);
		_vertexes[13].pos = float2(x_pos.y, y_pos.w);
		_vertexes[14].pos = float2(x_pos.z, y_pos.w);
		_vertexes[15].pos = float2(x_pos.w, y_pos.w);

		_vertexes[0].tc = float2(x_tc.x, y_tc.x);
		_vertexes[1].tc = float2(x_tc.y, y_tc.x);
		_vertexes[2].tc = float2(x_tc.z, y_tc.x);
		_vertexes[3].tc = float2(x_tc.w, y_tc.x);

		_vertexes[4].tc = float2(x_tc.x, y_tc.y);
		_vertexes[5].tc = float2(x_tc.y, y_tc.y);
		_vertexes[6].tc = float2(x_tc.z, y_tc.y);
		_vertexes[7].tc = float2(x_tc.w, y_tc.y);

		_vertexes[8].tc = float2(x_tc.x, y_tc.z);
		_vertexes[9].tc = float2(x_tc.y, y_tc.z);
		_vertexes[10].tc = float2(x_tc.z, y_tc.z);
		_vertexes[11].tc = float2(x_tc.w, y_tc.z);

		_vertexes[12].tc = float2(x_tc.x, y_tc.w);
		_vertexes[13].tc = float2(x_tc.y, y_tc.w);
		_vertexes[14].tc = float2(x_tc.z, y_tc.w);
		_vertexes[15].tc = float2(x_tc.w, y_tc.w);

		for (int i = 0; i < 16; i++)
		{
			float2 t = 2 * _vertexes[i].pos / c.window_size - float2(1, 1);
			_vertexes[i].pos = { t.x, -t.y };
			_vertexes[i].mulColor = item.mul_color;
			_vertexes[i].addColor = item.add_color;
		}

		if (textures_handles.size() == 512)
			flush(c);
	}

	void NinePatch::flush(Graphics::context& c)
	{
		if (vertexes.empty()) return;

		auto& graphics = c.command_list->get_graphics();
		graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);
		graphics.set_index_buffer(index_buffer->get_index_buffer_view());
		graphics.set_pipeline(current_state);

		auto data = c.command_list->place_data(sizeof(Vertex) * vertexes.size(), sizeof(Vertex));
		c.command_list->write<Vertex>(data, vertexes);
		auto view = data.resource->create_view<HAL::StructuredBufferView<Table::vertex_input>>(*c.command_list->frame_resources, StructuredBufferViewDesc{ (UINT)data.offset, (UINT)data.size,false });

		{
			Slots::NinePatch patch_data;
			patch_data.GetVb() = view.structuredBuffer;
			patch_data.GetTextures() = textures_handles;
			patch_data.set(graphics);
		}

		graphics.draw_indexed(9 * 2 * 3, 0, 0, UINT(vertexes.size() / 16));

		current_state = nullptr;
		vertexes.clear();
		textures_handles.clear();
	}

	void NinePatch::draw(Graphics::context& c, GUI::Texture& item, rect r)
	{
		draw(c, item, r, GetPSO<PSOS::NinePatch>());
	}

	void NinePatch::draw(Graphics::context& c, Graphics::PipelineState::ptr pipeline_state, rect r)
	{
		GUI::Texture item;
		draw(c, item, r, pipeline_state);
	}

}