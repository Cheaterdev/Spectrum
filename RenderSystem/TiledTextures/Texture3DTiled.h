#pragma once

class Texture3DTiled
{
protected:
	TileHeapManager heap_manager_static;

	std::vector<mip_info> mips;
	ivec3 one_tile_size;

	std::mutex m, place_mutex;
	bool is_good(ivec3 pos, int mip);
	Tile::ptr create_tile_internal(ivec3 position, int mip_level);
public:

	ivec3 get_tiles_count(int mip_level = 0);
	ivec3 get_voxels_per_tile();
	virtual int get_used_tiles();
	Tile::ptr& get_tile(int mip_level, ivec3 pos);


	bool exists(ivec3 pos, int mip_level);
	bool exists_parent(Tile::ptr& tile);
	Tile::ptr get_tile_safe(int mip_level, ivec3 pos);

	uint32_t get_max_tiles_count(int mip = 0);

	int get_index(ivec3 position, int mip);


	int get_child_count(Tile::ptr& tile);

	Tile::ptr create_tile(ivec3 position, int mip_level);

	Tile::ptr& get_parent_tile(Tile::ptr &tile);

	void remove_tile(ivec3 pos, int level);

	virtual void remove_tile(Tile::ptr &tile);



	virtual void place_tile(Tile::ptr &tile);
	std::function<void(Tile::ptr&)> on_tile_load;
	std::function<void(Tile::ptr&)> on_tile_remove;

	using ptr = std::shared_ptr<Texture3DTiled>;
	Render::Texture::ptr texture;


	virtual void remove_all();
	Texture3DTiled(CD3DX12_RESOURCE_DESC desc);

	void clear(Render::CommandList& list);

	VisibilityBufferUniversal::ptr create_visibility();
	void load_all();
	virtual void update();

};

class Texture3DTiledDynamic :public Texture3DTiled
{
	TileHeapManager heap_manager_dynamic;

public:
	using ptr = std::shared_ptr<Texture3DTiledDynamic>;
	Texture3DTiledDynamic(CD3DX12_RESOURCE_DESC desc);

	virtual int get_used_tiles();


	virtual void remove_all() override;

	virtual void place_tile(Tile::ptr &tile) override;

	void clear_static(Render::CommandList& list);

	void clear_dynamic(Render::CommandList& list);

	virtual void update();

	Tile::ptr create_dynamic_tile(ivec3 position, int mip_level);


	bool remove_dynamic_tile(ivec3 pos, int level);



	Tile::ptr create_static_tile(ivec3 position, int mip_level);

	bool remove_static_tile(ivec3 pos, int level);


	virtual 	void remove_tile(Tile::ptr &tile) override;


	void test();

};