export module Graphics:MeshAsset;


import Core;
import :SceneObject;
import :MaterialAsset;
import GUI;

import HAL;
import :Texture;
import :Quanta;
import :QuantaAsset;
import :QuantaResidency;

import stl.core;
export{
	class MeshAssetInstance;


	template <typename T>
	struct InlineMeshlet
	{
		unsigned int primitive_offset;
		unsigned int unique_offset;

		std::vector<T> UniqueVertexIndices;
		std::vector<T> PrimitiveIndices;

		Table::Meshes::MeshletCullData cull_data;

	private:
		SERIALIZE()
		{
			ar& NVP(UniqueVertexIndices);
			ar& NVP(PrimitiveIndices);
			ar& NVP(cull_data);

		}

	};


	struct MeshInfo
	{
		unsigned int vertex_offset;
		unsigned int vertex_count;
		size_t material;

		unsigned int index_offset;
		unsigned int index_count;
		std::shared_ptr<Primitive> primitive;

		unsigned int node_index;

		// Cluster LOD DAG: the first base_meshlet_count are the original clusters.
		std::vector<InlineMeshlet<UINT>> meshlets;
		unsigned int base_meshlet_count = 0;
		// LOD hierarchy levels; FirstCluster is relative to this mesh (see MeshletGroup).
		std::vector<Table::Meshes::MeshletGroup> lod_groups;
		std::vector<Table::Meshes::MeshletGroup> lod_nodes;



	private:
		SERIALIZE()
		{
			ar& NVP(vertex_offset);
			ar& NVP(vertex_count);
			ar& NVP(material);
			ar& NVP(index_offset);
			ar& NVP(index_count);
			ar& NVP(primitive);
			ar& NVP(node_index);
			ar& NVP(meshlets);
			ar& NVP(base_meshlet_count);
			ar& NVP(lod_groups);
			ar& NVP(lod_nodes);
		}
	};

	struct MeshNode
	{
		mat4x4 local_matrix;
		mat4x4 mesh_matrix;

		unsigned int mesh_id = -1;
		std::vector<MeshNode> childs;

		virtual void iterate(std::function<bool(MeshNode*)> f);

	private:
		SERIALIZE()
		{
			ar& NVP(local_matrix);
			ar& NVP(mesh_matrix);
			ar& NVP(mesh_id);
			ar& NVP(childs);
		}
	};

	class AssetLoadingContext
	{
	public:

		using ptr = std::shared_ptr<AssetLoadingContext>;

		GUI::user_interface::ptr ui;
		Task::ptr loading_task;


	};

	// What a part keeps of a range of the asset's buffer when nothing binds it: the buffer and the offsets, no descriptors.
	// It saves exactly as the StructuredBufferView it replaces (resource, then desc), so files saved with a view still load.
	struct BufferRange
	{
		HAL::Buffer::ptr resource;
		HAL::StructuredBufferViewDesc desc = {};

	private:
		SERIALIZE()
		{
			ar& NVP(resource);
			ar& NVP(desc);
		}
	};

	class CompiledMeshInfo
	{
	public:
		HAL::RaytracingAccelerationStructure::ptr ras;

		HAL::StructuredBufferView<Table::Meshes::mesh_vertex_input> vertex_buffer_view;
		HAL::StructuredBufferView<UINT32> index_buffer_view;
	// Only its offsets are used (instance setup), so no descriptors are made for it on load.
		BufferRange meshet_view;
		// Original clusters at the front of meshet_view; the rest are LOD levels.
		uint base_meshlet_count = 0;
		// This mesh's MeshletGroup ranges within MeshAsset::meshlet_groups.
		uint lod_group_offset = 0;
		uint lod_node_offset = 0;
		uint lod_node_count = 0;

		DrawIndexedArguments draw_arguments;
		DispatchMeshArguments dispatch_mesh_arguments;
		DispatchMeshArguments lod_dispatch_mesh_arguments;

		// Ray tracing's own triangles when the BLAS is built from one cut of the Quanta data instead of the full mesh
		// (MeshAsset::build_rt_proxies). Not serialized: rebuilt after the Quanta data is loaded. rt_error is the cut's
		// error threshold in the mesh's own units, how far the BLAS surface may sit from the one that is drawn.
		HAL::Buffer::ptr rt_buffer;
		HAL::StructuredBufferView<Table::Meshes::mesh_vertex_input> rt_vertex_buffer_view;
		HAL::StructuredBufferView<UINT32> rt_index_buffer_view;
		float rt_error = 0;

		uint node_index;
			size_t material;
				std::shared_ptr<Primitive> primitive;

	private:
		SERIALIZE()
		{
			ar& NVP(primitive);
			ar& NVP(material);
			ar& NVP(node_index);
			ar& NVP(vertex_buffer_view);
			ar& NVP(index_buffer_view);
			ar& NVP(meshet_view);
			ar& NVP(base_meshlet_count);
			ar& NVP(draw_arguments);
			ar& NVP(dispatch_mesh_arguments);
			ar& NVP(lod_group_offset);
			ar& NVP(lod_node_offset);
			ar& NVP(lod_node_count);
			ar& NVP(lod_dispatch_mesh_arguments);
		}
	};
	class MeshData : public loader<MeshData, std::string, AssetLoadingContext::ptr>
	{
	public:
		using ptr = std::shared_ptr<MeshData>;
		std::vector<Table::Meshes::mesh_vertex_input::Compiled> vertex_buffer;
		std::vector<UINT32> index_buffer;

		std::vector<MeshInfo> meshes;
		// Quanta (paged cluster geometry) for each of `meshes`, built at import. Separate from the
		// meshlets above, which the current renderer keeps using.
		std::vector<std::shared_ptr<Quanta::Mesh>> quanta;
		std::vector<MaterialAsset::ptr> materials;
		MeshNode root_node;
		std::shared_ptr<Primitive> primitive;

		static std::shared_ptr<MeshData> load_assimp(const std::string& file_name, resource_file_depender& files, AssetLoadingContext::ptr& context);
		static bool init_default_loaders();
		virtual void calculate_size();
	};
	class MeshAssetInstance;


	class MeshAsset : public Asset
	{
			MeshAsset();
		std::shared_ptr<MeshAssetInstance> preview_mesh;
	public:
		static const Asset_Type TYPE = Asset_Type::MESH;

		using ptr = s_ptr<MeshAsset>;
		using ref = AssetReference<MeshAsset>;

		HAL::Buffer::ptr buffer;


		TypedHandle<HAL::InstanceDesc> ras_handle;


		HAL::StructuredBufferView<Table::Meshes::mesh_vertex_input>		vertex_buffer_view;
		HAL::StructuredBufferView<UINT32>						index_buffer_view;
		HAL::StructuredBufferView<UINT32>						unique_indices;
		HAL::StructuredBufferView<UINT32>						primitive_indices;
		HAL::StructuredBufferView<Table::Meshes::Meshlet>				meshlets;
		HAL::StructuredBufferView<Table::Meshes::MeshletCullData>		meshlet_cull_datas;
		HAL::StructuredBufferView<Table::Meshes::MeshletGroup>		meshlet_groups;

		std::vector<CompiledMeshInfo>	meshes;
		// Quanta data per entry of `meshes`. Lives in its own "quanta" entry of the asset file (not in the
		// cereal stream of the asset itself), so assets saved before it existed still load, with this empty.
		std::vector<std::shared_ptr<Quanta::Mesh>> quanta;
		std::vector<MaterialAsset::ref> materials;

		std::shared_ptr<Primitive> primitive;
		mat4x4 local_transform;
		MeshNode root_node;
		std::vector<MeshNode*> nodes;

		virtual void shutdown() override;
		virtual void save_partitions(FileDataStorage& storage) override;
		virtual void load_partitions(FileDataStorage& storage, UniversalContext* context) override;
		// A loaded mesh keeps its Quanta pages in the asset file, and saving writes over that file: before_save() reads
		// the pages back while the old file is still there, after_save() points the live meshes at the new one.
		virtual void before_save() override;
		virtual void after_save(FileDataStorage& storage) override;
		MeshAsset(std::wstring, AssetLoadingContext::ptr = nullptr);
		virtual ~MeshAsset();
		std::shared_ptr<MeshAssetInstance> create_instance();
		virtual void try_register();

		virtual Asset_Type get_type();
		virtual void update_preview(HAL::Texture::ptr preview);
		virtual AssetStorage::ptr  register_new(std::wstring name = L"", Guid g = Guid());
		virtual void reload_resource() override;


		void init_gpu();

		// The BLAS builds are submitted when the asset loads and not waited for there. Whatever hands a BLAS to a TLAS
		// build (MeshAssetInstance::update_rtx_instance) waits here first, which is instant once the GPU is done.
		void wait_for_blas();

		// Replaces the BLAS of every part that has more triangles than Quanta::rt_triangle_budget() with one built from a
		// cut of its Quanta data. Needs `quanta`, so it runs after that is filled (import, or load_partitions).
		void build_rt_proxies();
	private:
		// Copies of `quanta` with their page bytes in memory, only between before_save() and after_save().
		std::vector<std::shared_ptr<Quanta::Mesh>> saving_quanta;

		HAL::FenceWaiter blas_ready;
		std::atomic<bool> blas_pending = false;
		// Structures replaced by build_rt_proxies while their build may still run on the GPU.
		std::vector<HAL::RaytracingAccelerationStructure::ptr> retired_ras;

		SERIALIZE()
		{
			SAVE_PARENT(Asset);
			{
				PROFILE(L"mesh_ser_primitive");
				ar& NVP(primitive);
			}
			{
				PROFILE(L"mesh_ser_parts");
				ar& NVP(meshes);
			}
			{
				PROFILE(L"mesh_ser_materials");
				ar& NVP(materials);
			}
			{
				PROFILE(L"mesh_ser_nodes");
				ar& NVP(local_transform);
				ar& NVP(root_node);
			}


			{
				PROFILE(L"mesh_ser_gpu_buffers");
				ar& NVP(buffer);
				ar& NVP(vertex_buffer_view);
				ar& NVP(index_buffer_view);
				ar& NVP(unique_indices);
				ar& NVP(primitive_indices);
				ar& NVP(meshlets);
				ar& NVP(meshlet_cull_datas);
				ar& NVP(meshlet_groups);

			}

			IF_LOAD()
			{
				{
					PROFILE(L"mesh_ser_collect_nodes");
					root_node.iterate([this](MeshNode* node)
						{
							nodes.push_back(node);

							return true;
						});
				}

				init_gpu();
			}


		}

	};

	class MeshAssetInstance : public scene_object, public AssetHolder, public Graphics::renderable
	{
		friend class mesh_renderer;
		friend class gpu_mesh_renderer;
		friend class gpu_cached_renderer;


		friend class stencil_renderer;

			Slots::Meshes::MeshInstanceInfo::Compiled mesh_instance_info;
		std::mutex m;
		void init_asset();
		void update_nodes();


		bool ras_inited = false;
		//std::vector<RaytracingAccelerationStructure::ptr> raytracing_as;

		TypedHandle<Table::Meshes::MeshCommandData::Compiled> meshpart_handle;
		TypedHandle<Table::Meshes::RaytraceInstanceInfo::Compiled> instance_handle;
		TypedHandle<HAL::InstanceDesc> ras_handle;

		size_t nodes_count;
		size_t rendering_count;
		HAL::StructuredBufferView<Table::Meshes::node_data> nodes_buffer_view;
		bool need_update_mats = false;
	public:
		MESH_TYPE type = MESH_TYPE::STATIC;
			// The scene's mesh id of rendering[0] (SceneData.meshes, command_ids); rendering[i] is this + i. Only
			// meaningful while the instance is in a scene.
			uint first_mesh_id() const { return static_cast<uint>(meshpart_handle.get_offset()); }
		MeshAsset::ref mesh_asset;

		struct mesh_asset_node
		{
			MeshNode* asset_node;
			unsigned int global_index;
			mesh_asset_node(MeshNode* node);
		};


		TypedHandle<Table::Meshes::node_data::Compiled> nodes_handle;
		// Every part's range in universal_meshlet_mask_manager (MeshInfo::
		// meshlet_mask_offset), one allocation for the whole instance.
		TypedHandle<uint> meshlet_mask_handle;
		struct render_info
		{

			std::shared_ptr<Primitive> primitive;
			std::shared_ptr<Primitive> primitive_global;
			Slots::Meshes::MeshInfo mesh_info;

			DrawIndexedArguments draw_arguments;
			DispatchMeshArguments dispatch_mesh_arguments;
			DispatchMeshArguments lod_dispatch_mesh_arguments;
			DispatchMeshArguments quanta_dispatch_mesh_arguments;   // one group per Quanta node, 0 without Quanta
			UINT material_id;
			MaterialAsset* material;
			UINT meshlet_offset;
			UINT meshlet_count;
			UINT node_id;
			mat4x4 global_mat;

			//compiled
			Slots::Meshes::MeshInfo::Compiled compiled_mesh_info;
			Slots::Meshes::MeshInstanceInfo::Compiled mesh_instance_info;
			HAL::RaytracingAccelerationStructure::ptr ras;

			HAL::StructuredBufferView<Table::Meshes::mesh_vertex_input> vertex_buffer_view;
			HAL::StructuredBufferView<UINT32> index_buffer_view;

		};

		// This instance's copy of each of mesh_asset->quanta in the shared GPU buffers (null where the
		// mesh has none), indexed like it. Held so the ranges live as long as the instance draws them.
		std::vector<std::shared_ptr<Quanta::Resident>> quanta_residents;

		std::vector<render_info> rendering;
		std::vector<mesh_asset_node> nodes;
		std::vector<int> nodes_indexes;

		std::vector<MaterialAsset::ref> overrided_material;



		//std::vector<RaytracingAccelerationStructure::ptr> create_raytracing_as();


		bool init_ras(HAL::CommandList::ptr list);
		using ptr = s_ptr<MeshAssetInstance>;
		virtual ~MeshAssetInstance();
		MeshAssetInstance(MeshAsset::ptr asset);
		void draw(MeshRenderContext::ptr context);

		void override_material(size_t i, MaterialAsset::ptr mat);

		virtual void on_asset_change(Asset::ptr asset);


		virtual void on_add(scene_object* parent) override;

		virtual void on_remove() override;
		virtual	bool update_transforms()override;

		void update_rtx_instance();
	private:

		MeshAssetInstance();

		SERIALIZE();

	};

	class universal_rtx_manager :public Singleton<universal_rtx_manager>, public HAL::virtual_gpu_buffer<Table::Meshes::RaytraceInstanceInfo>
	{
		static const size_t MAX_NODES_SIZE = 100_mb / sizeof(Table::Meshes::RaytraceInstanceInfo);
	public:
		universal_rtx_manager();
	};

	class universal_mesh_instance_manager :public Singleton<universal_mesh_instance_manager>, public HAL::virtual_gpu_buffer<Table::Meshes::MeshInstanceInfo>
	{
		static const size_t MAX_NODES_SIZE = 100_mb / sizeof(Table::Meshes::MeshInstanceInfo);
	public:
		universal_mesh_instance_manager();
	};


	class universal_nodes_manager :public Singleton<universal_nodes_manager>, public HAL::virtual_gpu_buffer<Table::Meshes::node_data>
	{
		static const size_t MAX_NODES_SIZE = 100_mb / sizeof(Table::Meshes::node_data);
	public:
		universal_nodes_manager();
	};


	// Per-meshlet cull results the debug view's capture records (see
	// MeshletCaptureWrite in meshrender.prism): one frame-tagged entry per
	// cluster. Written only while the debug view captures.
	class universal_meshlet_mask_manager :public Singleton<universal_meshlet_mask_manager>, public HAL::virtual_gpu_buffer<uint>
	{
		static const size_t MAX_WORDS = 64_mb / sizeof(uint);
	public:
		universal_meshlet_mask_manager();

		static uint words_for(uint meshlet_count) { return meshlet_count; }
	};

	class universal_material_info_part_manager :public Singleton<universal_material_info_part_manager>, public HAL::virtual_gpu_buffer<Table::Meshes::MaterialCommandData>
	{
		static const size_t MAX_COMMANDS_SIZE = 100_mb / sizeof(Table::Meshes::MaterialCommandData);
	public:
		universal_material_info_part_manager();
	};

	class SceneFrameManager :public Singleton<SceneFrameManager>
	{

	public:
		void prepare(HAL::CommandList::ptr& command_list, Scene& scene);
	};







	//BOOST_CLASS_TRACKING(material_holder, boost::serialization::track_never);
	// REGISTER_TYPE(MeshAssetInstance);
	// REGISTER_TYPE(MeshAsset);
	// REGISTER_TYPE(AssetReference<MeshAsset>);

}
