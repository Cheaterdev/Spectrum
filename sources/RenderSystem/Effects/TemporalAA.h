#pragma once


using namespace Graphics;

class TemporalAA
{
	std::vector<Texture::ptr> textures;
	std::vector<float2> offsets;
	std::vector<float4> subsamples;

	Texture::ptr current;
	Texture::ptr prev;
	int current_index;

	UINT all_count = 1;
public:

	void set(Graphics::CommandList::ptr& list, int slot)
	{

		for (UINT i = 0; i < textures.size(); i++)
		{
			////				list->transition(textures[i], Graphics::ResourceState::PIXEL_SHADER_RESOURCE);

			((SignatureDataSetter*)&list->get_graphics())->set_dynamic(slot, i, textures[std::min(all_count - 1, i)]->texture_2d()->get_static_srv());
		}
		// list->get_graphics().set_dynamic(slot, 0, current->texture_2d()->get_static_srv());
		//list->get_graphics().set_dynamic(slot, 1, prev->texture_2d()->get_static_srv());
	}

	TemporalAA()
	{
		/* offsets =
		 {
			 { 0.0588235, 0.419608},
			 {0.298039, 0.180392},
			 {0.180392, 0.819608},
			 {0.419608, 0.698039},
			 {0.580392, 0.298039},
			 { 0.941176, 0.0588235},
			 {0.698039, 0.941176},
			 {0.819608, 0.580392}
		 };*/
		 /* offsets =
		  {
			  { 0.25f, -0.25f },
			  { -0.25f, 0.25f },
			  { 0.25f, 0.25f },
			  { -0.25f, -0.25f }
		  };*/
		  //  offsets = {float2(0.25, -0.25), float2(-0.25,  0.25) };
		  //  subsamples = { float4(1, 1, 1, 0), float4(2, 2, 2, 0) };

		offsets = { float2(0,0) };
		subsamples = { float4(0,0) };
	}
	//   HandleTable srv_table;
	void resize(ivec2 size)
	{
		textures.clear();
		current_index = 0;
		all_count = 0;
		//  srv_table = DescriptorHeapManager::get().get_csu()->create_table(4);

		for (UINT i = 0; i < offsets.size(); i++)
		{
			textures.emplace_back(new Graphics::Texture(HAL::ResourceDesc::Tex2D(Graphics::Format::R16G16B16A16_FLOAT, { size.x, size.y }, 1, 0, HAL::ResFlags::RenderTarget | HAL::ResFlags::UnorderedAccess), Graphics::ResourceState::PIXEL_SHADER_RESOURCE));

			textures[i]->set_name(std::string("TemporalAA_") + std::to_string(i));
			//      srv_table[i] = textures.back()->texture_2d()->srv();
		}
	}


	void make_current(Texture::ptr& _current)
	{
		swap(textures[current_index], _current);
		prev = this->current;
		this->current = textures[current_index];

		if (!prev)
			prev = this->current;
		if (all_count < textures.size())
			all_count++;
		if (++current_index == textures.size()) current_index = 0;
	}

	Texture::ptr& get_current()
	{
		return textures[current_index];
	}


	float2 get_current_offset()
	{
		return offsets[current_index];
	}

	float2 get_current_subsample()
	{
		return subsamples[current_index];
	}
};