export module Graphics:MeshAsset;
import <RenderSystem.h>;

import Core;
import :SceneObject;
import :MaterialAsset;
import GUI;

import HAL;


import <stl/core.h>;
export{
	class MeshAssetInstance;


	template <typename T>
	struct InlineMeshlet
	{
		unsigned int primitive_offset;
		unsigned int unique_offset;

		std::vector<T> UniqueVertexIndices;
		std::vector<T> PrimitiveIndices;

		Table::MeshletCullData cull_data;

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

		std::vector<InlineMeshlet<UINT>> meshlets;


		
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
		}
	};

	struct MeshNode
	{
		mat4x4 local_matrix;
		mat4x4 mesh_matrix;

		unsigned int mesh_id = -1;
		std::vector<MeshNode> childs;

		virtual void iterate(std::function<bool(MeshNode*)> f)
		{
			if (f(this))
			{
				for (auto& c : childs)
					c.iterate(f);
			}
		}

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

	class CompiledMeshInfo
	{
	public:
		HAL::RaytracingAccelerationStructure::ptr ras;

		HAL::StructuredBufferView<Table::mesh_vertex_input> vertex_buffer_view;
		HAL::StructuredBufferView<UINT32> index_buffer_view;
	HAL::StructuredBufferView<Table::Meshlet> meshet_view;

		DrawIndexedArguments draw_arguments;
		DispatchMeshArguments dispatch_mesh_arguments;

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
			ar& NVP(draw_arguments);
			ar& NVP(dispatch_mesh_arguments);
		}
	};
	class MeshData : public loader<MeshData, std::string, AssetLoadingContext::ptr>
	{
	public:
		using ptr = std::shared_ptr<MeshData>;
		std::vector<Table::mesh_vertex_input::Compiled> vertex_buffer;
		std::vector<UINT32> index_buffer;

		std::vector<MeshInfo> meshes;
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
		LEAK_TEST(MeshAsset)

			MeshAsset();
		std::shared_ptr<MeshAssetInstance> preview_mesh;
	public:
		static const Asset_Type TYPE = Asset_Type::MESH;

		using ptr = s_ptr<MeshAsset>;
		using ref = AssetReference<MeshAsset>;

		HAL::Resource::ptr buffer;


		TypedHandle<D3D12_RAYTRACING_INSTANCE_DESC> ras_handle;


		HAL::StructuredBufferView<Table::mesh_vertex_input>		vertex_buffer_view;
		HAL::StructuredBufferView<UINT32>						index_buffer_view;
		HAL::StructuredBufferView<UINT32>						unique_indices;
		HAL::StructuredBufferView<UINT32>						primitive_indices;
		HAL::StructuredBufferView<Table::Meshlet>				meshlets;
		HAL::StructuredBufferView<Table::MeshletCullData>		meshlet_cull_datas;

		std::vector<CompiledMeshInfo>	meshes;
		std::vector<MaterialAsset::ref> materials;

		std::shared_ptr<Primitive> primitive;
		mat4x4 local_transform;
		MeshNode root_node;
		std::vector<MeshNode*> nodes;

		virtual void shutdown() override
		{
			preview_mesh.reset();
		}
		MeshAsset(std::wstring, AssetLoadingContext::ptr = nullptr);
		virtual ~MeshAsset()
		{
			materials.clear();
		}
		std::shared_ptr<MeshAssetInstance> create_instance();
		virtual void try_register();

		virtual Asset_Type get_type();
		virtual void update_preview(HAL::Texture::ptr preview);
		virtual AssetStorage::ptr  register_new(std::wstring name = L"", Guid g = Guid());
		virtual void reload_resource() override { }


		void init_gpu();
	private:
		SERIALIZE()
		{
			SAVE_PARENT(Asset);
			ar& NVP(primitive);
			ar& NVP(meshes);
			ar& NVP(materials);
			ar& NVP(local_transform);
			ar& NVP(root_node);


			{
				ar& NVP(buffer);
				ar& NVP(vertex_buffer_view);
				ar& NVP(index_buffer_view);
				ar& NVP(unique_indices);
				ar& NVP(primitive_indices);
				ar& NVP(meshlets);
				ar& NVP(meshlet_cull_datas);

			}

			IF_LOAD()
			{
				root_node.iterate([this](MeshNode* node)
					{
						nodes.push_back(node);

						return true;
					});

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


		LEAK_TEST(MeshAssetInstance)

			Slots::MeshInstanceInfo::Compiled mesh_instance_info;
		std::mutex m;
		void init_asset();
		void update_nodes();


		bool ras_inited = false;
		//std::vector<RaytracingAccelerationStructure::ptr> raytracing_as;

		TypedHandle<Table::MeshCommandData::Compiled> meshpart_handle;
		TypedHandle<Table::RaytraceInstanceInfo::Compiled> instance_handle;
		TypedHandle<D3D12_RAYTRACING_INSTANCE_DESC> ras_handle;

		size_t nodes_count;
		size_t rendering_count;
		HAL::StructuredBufferView<Table::node_data> nodes_buffer_view;
		bool need_update_mats = false;
	public:
		MESH_TYPE type = MESH_TYPE::STATIC;
		MeshAsset::ref mesh_asset;

		struct mesh_asset_node
		{
			MeshNode* asset_node;
			unsigned int global_index;
			mesh_asset_node(MeshNode* node)
			{
				asset_node = node;
			}
		};


		TypedHandle<Table::node_data::Compiled> nodes_handle;
		struct render_info
		{

			std::shared_ptr<Primitive> primitive;
			std::shared_ptr<Primitive> primitive_global;
			Slots::MeshInfo mesh_info;

			DrawIndexedArguments draw_arguments;
			DispatchMeshArguments dispatch_mesh_arguments;
			UINT material_id;
			MaterialAsset* material;
			UINT meshlet_offset;
			UINT meshlet_count;
			UINT node_id;
			mat4x4 global_mat;

			//compiled
			Slots::MeshInfo::Compiled compiled_mesh_info;
			Slots::MeshInstanceInfo::Compiled mesh_instance_info;
			HAL::RaytracingAccelerationStructure::ptr ras;

			HAL::StructuredBufferView<Table::mesh_vertex_input> vertex_buffer_view;
			HAL::StructuredBufferView<UINT32> index_buffer_view;

		};

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

	class universal_rtx_manager :public Singleton<universal_rtx_manager>, public HAL::virtual_gpu_buffer<Table::RaytraceInstanceInfo>
	{
		static const size_t MAX_NODES_SIZE = 100_mb / sizeof(Table::RaytraceInstanceInfo);
	public:
		universal_rtx_manager() :HAL::virtual_gpu_buffer<Table::RaytraceInstanceInfo>(MAX_NODES_SIZE)
		{

		}
	};

	class universal_mesh_instance_manager :public Singleton<universal_mesh_instance_manager>, public HAL::virtual_gpu_buffer<Table::MeshInstanceInfo>
	{
		static const size_t MAX_NODES_SIZE = 100_mb / sizeof(Table::MeshInstanceInfo);
	public:
		universal_mesh_instance_manager() :HAL::virtual_gpu_buffer<Table::MeshInstanceInfo>(MAX_NODES_SIZE)
		{

		}
	};


	class universal_nodes_manager :public Singleton<universal_nodes_manager>, public HAL::virtual_gpu_buffer<Table::node_data>
	{
		static const size_t MAX_NODES_SIZE = 100_mb / sizeof(Table::node_data);
	public:
		universal_nodes_manager() :HAL::virtual_gpu_buffer<Table::node_data>(MAX_NODES_SIZE)
		{

		}
	};


	class universal_material_info_part_manager :public Singleton<universal_material_info_part_manager>, public HAL::virtual_gpu_buffer<Table::MaterialCommandData>
	{
		static const size_t MAX_COMMANDS_SIZE = 100_mb / sizeof(Table::MaterialCommandData);
	public:
		universal_material_info_part_manager() :HAL::virtual_gpu_buffer<Table::MaterialCommandData>(MAX_COMMANDS_SIZE)
		{

		}
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
