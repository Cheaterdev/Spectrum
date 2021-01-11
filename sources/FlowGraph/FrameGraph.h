#pragma once
#include <Data/Data.h>
#include "Math/Vectors.h"
#include "DX12/ResourceViews.h"
#include "DX12/Texture.h"
#include "Threads/Scheduler.h"

enum class ResourceType :int {
	UNSPECIFIED,
	Buffer,
	Texture,
	Heap

	, GENERATE_OPS
};

enum class ResourceFlags :int {
	None = 0,
	PixelRead = 1,
	ComputeRead = 2,
	DSRead = 1024,

	UnorderedAccess = 4,
	RenderTarget = 8,
	DepthStencil = 16,


	GenCPU = 32,
	ReadCPU = 64,

	Temporal = 0,
	Static = 128,
	Required = 256,

	Cube = 512,
	Changed = 1024

	, GENERATE_OPS
};

static const ResourceFlags WRITEABLE_FLAGS = ResourceFlags::UnorderedAccess | ResourceFlags::RenderTarget | ResourceFlags::DepthStencil | ResourceFlags::GenCPU;

struct BufferDesc
{
	size_t size;

	auto   operator<=>(const  BufferDesc& r)  const = default;
};

struct TextureDesc
{
	ivec3 size;
	DXGI_FORMAT format;
	UINT array_count;

	auto   operator<=>(const  TextureDesc& r)  const = default;
};


struct ResourceAllocInfo;

struct ResourceHandler
{
	bool is_new();
	void changed();

	bool is_changed();
	public:
	friend struct TaskBuilder;
	friend struct FrameContext;
	ResourceAllocInfo* info;
};


struct Pass;

struct UsedResources
{
	std::set<ResourceHandler*> resources;

	std::map<ResourceHandler*, ResourceFlags> resource_flags;
	std::set<ResourceHandler*> resource_creations;
};

struct ResourceAllocInfo
{
	std::string name;
// desc
	ResourceType type;
	MyVariant desc;
	ResourceFlags flags;
	bool placed;

	Render::ResourceHandle alloc_ptr;
	ResourceAllocInfo* orig = nullptr;

	CD3DX12_RESOURCE_DESC d3ddesc;
	Render::HeapType heap_type;
// setup
	Pass* valid_from = nullptr;
	Pass* valid_to = nullptr;
	Pass* valid_to_start = nullptr;



	bool enabled = true;

	bool is_new = false;
	std::list<Pass*> writers;
	std::list<Pass*> readers;

//compile
	std::map<Render::ResourceHandle, Render::Resource::ptr> resource_places;
	Render::Resource::ptr resource;

	Render::TextureView texture;
	Render::BufferView buffer;

	ResourceHandler* handler;

	bool need_recreate = false;
	bool passed = false;
	size_t frame_id;

	Pass* get_last_writer()
	{
		if (writers.empty()) return nullptr;

		return writers.back();
	}
};


struct TaskBuilder
{

	std::map<std::string, ResourceHandler> resources;
	std::map<std::string, std::string> resources_names;

	std::map<ResourceHandler*, ResourceAllocInfo> alloc_resources;

	std::set<ResourceHandler*> passed_resources;

	Render::ResourceHeapAllocator allocator;
	Render::ResourceHeapAllocator static_allocator;

//	Render::PlacedAllocator allocator;
//	Render::PlacedAllocator static_allocator;

	Render::FrameResourceManager frames;
	Render::FrameResources::ptr current_frame;

	std::map<int, Render::ResourceHeapAllocator> frame_allocs;

	Render::ResourceHeapAllocator* current_alloc;
	Pass* current_pass;
	void begin(Pass* pass);

	void end(Pass* pass);

	ResourceHandler* create_texture(std::string name, ivec2 size, UINT array_count, DXGI_FORMAT format, ResourceFlags flags = ResourceFlags::None);
	ResourceHandler* need_texture(std::string name, ResourceFlags flags = ResourceFlags::PixelRead);
	ResourceHandler* recreate_texture(std::string name, ResourceFlags flags = ResourceFlags::PixelRead);

	ResourceHandler* create_buffer(std::string name, UINT64 size, ResourceFlags flags = ResourceFlags::None);
	ResourceHandler* need_buffer(std::string name, ResourceFlags flags = ResourceFlags::PixelRead);


	//void free_texture(ResourceHandler* handler);
	void pass_texture(std::string name, Render::TextureView tex, ResourceFlags flags = ResourceFlags::None);
	void pass_texture(std::string name, Render::Texture::ptr tex, ResourceFlags flags = ResourceFlags::None);

	Render::TextureView request_texture(ResourceHandler* handler);
	void create_resources();
	void reset();


};



struct FrameContext
{
	Pass* pass;
	Render::FrameResources::ptr frame;

	std::list<Render::ResourceView> textureViews;

	Render::TextureView get_texture(ResourceHandler* holder);
	Render::BufferView get_buffer(ResourceHandler* holder);

//	void request_resources(UsedResources& resources, TaskBuilder& builder);

	Render::CommandList::ptr list;

	Render::CommandList::ptr& get_list();
	void begin(Pass* pass, Render::FrameResources::ptr& frame);

	void end();


	void execute();

	void register_subview(const Render::ResourceView& view)
	{
		textureViews.push_back(view);
	}
};


enum class PassFlags
{
	General = 0,
	Required =  1,

	Graphics = 0,
	Compute = 2,

	GENERATE_OPS
};


struct Pass
{
	int id = 0;
	int call_id;
	int dependency_level;
	bool enabled = true;
	bool renderable = true;
	PassFlags flags;
	std::string name;
	UsedResources used;
	FrameContext context;
	std::set<Pass*> related;
	std::future<void> render_task;
	Render::FenceWaiter fence_end;

	int graphic_count = 0;
	int compute_count = 0;
	Pass* wait_pass = nullptr;
	Pass* prev_pass = nullptr;
	virtual bool setup(TaskBuilder& builder) = 0;

	Render::CommandListType get_type()
	{
		Render::CommandListType type = Render::CommandListType::DIRECT;

		if (check(flags & PassFlags::Compute))
			type = Render::CommandListType::COMPUTE;

		return type;
	}
	void compile(TaskBuilder& builder);

	virtual void render(Render::FrameResources::ptr& frame) = 0;
	void wait();
	void execute();

	bool active()
	{
		return enabled && renderable;
	}
};


template <class T>
struct TypedPass : public Pass
{

	using render_func_type = std::function<void(T&, FrameContext&)>;
	using setup_func_type = std::function<bool(T&, TaskBuilder&)>;
	using setup_func_type_void = std::function<void(T&, TaskBuilder&)>;

	
	T data;
	setup_func_type setup_func;
	render_func_type render_func;

	TypedPass(int id, std::string name, setup_func_type s, render_func_type r)
	{
		this->id = id;
		this->name = name;

		setup_func = s;
		render_func = r;
	}

	virtual bool setup(TaskBuilder& builder) override
	{
		builder.begin(this);
		bool res = setup_func(data, builder);
		builder.end(this);

		return res;
	}

	virtual void render(Render::FrameResources::ptr& frame) override
	{
		if (!enabled || !renderable)  return;
		render_task = scheduler::get().enqueue([this, &frame]() {
			context.begin(this, frame);
			render_func(data, context);
			context.end();
			}, std::chrono::steady_clock::now());

	//	render_task.wait();
	}
};
class camera;
class main_renderer;
class Scene;

struct CreationContext
{
	ivec2 frame_size;
	camera* cam;
	main_renderer* renderer;
	Scene* scene;

	float time;
	float totalTime = 0;
	float3 sunDir;
};

//ENABLE_ENUM(wtf);

class FrameGraph: public CreationContext
{
public:
	std::list<std::shared_ptr<Pass>> passes;

	std::list<std::shared_ptr<Pass>> required_passes;
	std::list<Pass*> enabled_passes;

public:
	TaskBuilder builder;
	bool optimize = true;
	template<class T>
	void pass(std::string name, typename TypedPass<T>::setup_func_type s, typename TypedPass<T>::render_func_type r, PassFlags flags = PassFlags::General)
	{
		passes.push_back(std::make_shared<TypedPass<T>>((UINT)passes.size(), name, s, r));
		passes.back()->flags = flags;

		if (check(flags & PassFlags::Required))
		{
			required_passes.push_back(passes.back());
		}
	}

	template<class T>
	void add_pass(std::string name, typename TypedPass<T>::setup_func_type_void s, typename TypedPass<T>::render_func_type r, PassFlags flags = PassFlags::General)
	{
		typename TypedPass<T>::setup_func_type f = [s](T& t, TaskBuilder& b) {s(t, b); return true; };
		
		pass<T>(name, f, r, flags);
	}

	void start_new_frame();

	void setup();
	void compile(int frame);
	void render();

	void reset();
};

class FrameGraphGenerator
{
public:
	virtual void generate(FrameGraph& graph) = 0;
};