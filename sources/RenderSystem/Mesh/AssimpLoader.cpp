#include "pch_render.h"

#include "Simplifyer/Simplifyer.h"
#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>
#include <assimp\DefaultLogger.hpp>
#include <assimp\LogStream.hpp>
#include <assimp\ProgressHandler.hpp>
#include <assimp\IOStream.hpp>
#include <assimp\IOSystem.hpp>

#include "Mesh/MeshletGeneration.h"
#include "Materials/universal_material.h"
#include "Assets/MeshAsset.h"

#include "GUI/Elements/ValueBox.h"
#include "GUI/Elements/ScrollContainer.h"
#include "GUI/Elements/CheckBoxText.h"
#include "GUI/Elements/Window.h"

struct MeshLoadingSettings
{
	float scale = 1;
	bool materials_remove = true;
	std::map<std::filesystem::path, AssetStorage::ptr> load_textures;

};
class LoadingWindow : public GUI::Elements::window
{
	MeshLoadingSettings settings;
public:
	std::promise<MeshLoadingSettings> result;
	LoadingWindow(std::string name, MeshLoadingSettings & _settings):settings(_settings)
	{

		set_title("loading " + name);

		docking = GUI::dock::FILL;
		width_size = GUI::size_type::MATCH_CHILDREN;
		height_size = GUI::size_type::MATCH_CHILDREN;
	
		contents->width_size = GUI::size_type::MATCH_CHILDREN;
		contents->height_size = GUI::size_type::MATCH_CHILDREN;



		auto left_panel = std::make_shared<GUI::base>();

		left_panel->width_size = GUI::size_type::MATCH_CHILDREN;
		left_panel->height_size = GUI::size_type::MATCH_CHILDREN;
		left_panel->docking = GUI::dock::LEFT;
		left_panel->y_type = GUI::pos_y_type::TOP;

		add_child(left_panel);




		auto s = std::make_shared<GUI::Elements::value_box>();
		s->min_value = 0;
		s->max_value = 1000;
		s->docking = GUI::dock::TOP;
		s->x_type = GUI::pos_x_type::LEFT;

		s->on_change = [this](int value, GUI::Elements::value_box * b)
		{
			settings.scale = float(value) / 20;
			//uniform->value.f_value = float(value) / 100;
			b->info->text = std::to_string(settings.scale).substr(0, 4);
			//uniform->on_change(uniform.get());
	
		};


		left_panel->add_child(s);



		auto c = std::make_shared<GUI::Elements::check_box_text>();

		c->docking = GUI::dock::TOP;

		c->x_type = GUI::pos_x_type::LEFT;

		c->get_label()->text = "unique mats";

		c->get_check()->set_checked(true);
		c->on_check = [this](bool v) {
			settings.materials_remove = v;
		};
		left_panel->add_child(c);



		auto ok = std::make_shared<GUI::Elements::button>();
		ok->docking = GUI::dock::TOP;

		ok->width_size = GUI::size_type::FIXED;
		ok->height_size = GUI::size_type::FIXED;
		ok->size = { 80,25 };

		ok->on_click = [this](GUI::Elements::button::ptr) {

		//	prom->set_value(float(s->current_value) / 20);
			result.set_value(settings);

			//prom_materials->set_value(float(c->is_checked()) / 20);
		remove_from_parent();


		};

		left_panel->add_child(ok);



		auto list = std::make_shared<GUI::Elements::scroll_container>();

		list->docking = GUI::dock::LEFT;
		list->size = {200,200};

		add_child(list);




		for (auto &p : settings.load_textures)
		{
			if (!p.second) continue;
			auto item = std::make_shared<GUI::Elements::check_box_text>();

			item->docking = GUI::dock::TOP;
			item->x_type = GUI::pos_x_type::LEFT;
			auto file_name = p.first.filename();

			
			item->get_label()->text = file_name.generic_string();

			item->get_check()->set_checked(true);

	//		auto & second = p.second;

			auto second_val = p.second;
			auto first_val = p.first;
			item->on_check = [this,first_val, second_val](bool v) {
				settings.load_textures[first_val] = v ? second_val : nullptr;
			};
			list->add_child(item);
		}
	}};
// My own implementation of IOStream
class MyIOStream : public Assimp::IOStream
{
        friend class MyIOSystem;
        std::string data;

        size_t pos;
    protected:
        // Constructor protected for private usage by MyIOSystem
        MyIOStream(std::string filename)
        {
            pos = 0;
            auto file = FileSystem::get().get_file(filename);

            if (file)
                data = file->load_all();
        }
    public:

        size_t Read(void* pvBuffer, size_t pSize, size_t pCount)
        {
            size_t count = std::min(data.size() - pos, pSize*pCount);
            memcpy(pvBuffer, data.data() + pos, count);
            pos += pSize*pCount;
            return count;
        }


        size_t Write(const void* pvBuffer, size_t pSize, size_t pCount) { return pCount; }
        aiReturn Seek(size_t pOffset, aiOrigin pOrigin)
        {
            if (pOrigin == aiOrigin::aiOrigin_CUR)
                pos += pOffset;

            if (pOrigin == aiOrigin::aiOrigin_END)
                pos += pOffset;

            if (pOrigin == aiOrigin::aiOrigin_SET)
                pos = pOffset;

            return aiReturn_SUCCESS;
        }
        size_t Tell() const { return pos; }
        size_t FileSize() const { return data.size(); }
        void Flush() {  }
};
// Fisher Price - My First Filesystem
class MyIOSystem : public Assimp::IOSystem
{
        resource_file_depender& files;

		std::filesystem::path path;
        // Check whether a specific file exists
        bool Exists(const char* pFile) const
        {
            return !!FileSystem::get().get_file(path / pFile);
        }
        // Get the path delimiter character we'd like to see
        virtual char getOsSeparator() const override
        {
            return '/';
        }
        // ... and finally a method to open a custom stream
        virtual Assimp::IOStream* Open(const char* pFile, const char* pMode = "rb") override
        {
			auto new_path = path / pFile;
			new_path = std::filesystem::canonical(new_path);
            auto file = FileSystem::get().get_file(new_path);
            files.add_depend(file);
            return new MyIOStream(new_path.generic_string());
        }

        virtual void Close(Assimp::IOStream* pFile) override
        {
            delete pFile;
        }

    public:
        MyIOSystem(std::filesystem::path path, resource_file_depender& _files) : files(_files)
        {
            this->path = path;
        }
};

class MyProgressHandler :public Assimp::ProgressHandler
{
	Task::ptr loading_task;
public:
	MyProgressHandler(AssetLoadingContext::ptr & context):loading_task(context->loading_task)
	{

	}

	virtual bool Update(float percentage = -1.f) override
	{
	
		loading_task->percent = percentage;

		return loading_task->active;
	}

};


std::shared_ptr<MeshData> MeshData::load_assimp(const std::string& file_name, resource_file_depender& files, AssetLoadingContext::ptr & context)
{
    Assimp::Importer importer;
	std::filesystem::path directory = std::filesystem::path(file_name).parent_path();

 //   if (directory.size())
   //     directory += std::string("\\");

	// TODO: need delete?
    importer.SetIOHandler(new MyIOSystem(directory, files));
	importer.SetProgressHandler(new MyProgressHandler(context));
    //  auto file = FileSystem::get().get_file(file_name);
    const aiScene* scene;

    try
    {
        scene = importer.ReadFile(std::filesystem::path(file_name).filename().generic_string(), aiProcess_JoinIdenticalVertices| aiProcess_GenBoundingBoxes| aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals/* | aiProcess_PreTransformVertices*/);
    }

    catch (const std::exception &e)
    {
        Log::get() << Log::LEVEL_ERROR  << "Mesh [" << file_name << "] loading error : " << e.what() << Log::endl;
        return nullptr;
    }


    std::string str = importer.GetErrorString();

    if (!scene)
        Log::get() << Log::LEVEL_ERROR << "Mesh [" << file_name << "] loading error : " << str << Log::endl;

	if (scene)
	{
		if (!scene->mNumMeshes)
			return nullptr;
		MeshLoadingSettings settings;

		std::map<std::filesystem::path, AssetStorage::ptr>& load_textures =settings.load_textures;


		auto check_texture = [&load_textures](std::filesystem::path name)
		{
			auto it = load_textures.find(name);

			if (it == load_textures.end())
			{
				auto file_name = name.filename();

				load_textures[name] = AssetManager::get().find_storage_by_name(file_name.generic_wstring());



			}
		};

		auto check_assimp_texture = [&directory, &check_texture](aiMaterial*native_material, aiTextureType type) {
			aiString path;
			if (AI_SUCCESS == native_material->GetTexture(type, 0, &path))
			{
				auto native_path = directory / std::string(path.C_Str());


				check_texture(native_path);
			}

		};


		for (unsigned int i = 0; i < scene->mNumMaterials; i++)
		{
			auto& native_material = scene->mMaterials[i];

			check_assimp_texture(native_material, aiTextureType_DIFFUSE);
			check_assimp_texture(native_material, aiTextureType_NORMALS);
			check_assimp_texture(native_material, aiTextureType_HEIGHT);

		}


	
		if (context&&context->ui)
		{


			

			auto wnd = std::make_shared<LoadingWindow>(file_name, settings);

			context->ui->add_task([context, wnd]() {context->ui->add_child(wnd); });

			settings = wnd->result.get_future().get();

			if (settings.materials_remove)
				scene = importer.ApplyPostProcessing(aiProcess_RemoveRedundantMaterials);

		}

        auto scale = settings.scale;
		{
			std::vector<std::future<void>> tasks;


			for (auto &t : load_textures)
			{

				if (t.second) continue;


				auto &&val = t.second;
				auto index = t.first;

				tasks.emplace_back(thread_pool::get().enqueue([t, index,&load_textures]() {


					auto tex = new TextureAsset(t.first);
					load_textures[index] = tex->register_new();
				}));

		

			}

			for (auto && e : tasks)
				e.wait();
		}

        std::shared_ptr<MeshData> result(new MeshData);
        std::mutex m;
      /*  Cache<std::wstring, TextureAsset::ptr> textures([](const std::wstring & name)
        {
            auto tex = new TextureAsset(name);      tex->register_new();
            return tex->get_ptr<TextureAsset>();
        });*/
        unsigned int vertex_count = 0;
        unsigned	int index_count = 0;

        for (unsigned int i = 0; i < scene->mNumMeshes; i++)
        {
            vertex_count += scene->mMeshes[i]->mNumVertices;
            index_count += scene->mMeshes[i]->mNumFaces * 3;
        }

        std::vector<Table::mesh_vertex_input::CB> vertices(vertex_count);
        std::vector<UINT32> indices(index_count);
        vertex_count = 0;
        index_count = 0;
        auto get_texture = [&load_textures, &m](std::filesystem::path name)->TextureAsset::ptr
        {
            return load_textures[name]->get_asset()->get_ptr<TextureAsset>();
        };
        std::vector<std::future<bool>> tasks;

		std::vector<MaterialAsset::ptr> materials;

		materials.resize(scene->mNumMaterials);

        result->materials.resize(scene->mNumMeshes);

        for (unsigned int i = 0; i < scene->mNumMaterials; i++)
        {
            tasks.emplace_back(thread_pool::get().enqueue([&materials ,scene,i,directory, &get_texture]()
            {
                auto& native_material = scene->mMaterials[i];
                aiString path;
                MaterialGraph::ptr graph(new MaterialGraph);
                TextureNode::ptr  tex_node;

                if (AI_SUCCESS == native_material->GetTexture(aiTextureType_DIFFUSE, 0, &path))
                {
                   auto native_path = directory / std::string(path.C_Str());
                    auto diff = get_texture(native_path);
                    tex_node = std::make_shared<TextureNode>(diff, true);
                    graph->register_node(tex_node);
                    graph->get_texcoord()->link(tex_node->get_input(0));
                    tex_node->get_output(0)->link(graph->get_base_color());
                }

                else
                {
                    aiColor3D albedo;
                    native_material->Get(AI_MATKEY_COLOR_DIFFUSE, albedo);
                    auto value_node = std::make_shared<VectorNode>(vec4(albedo.r, albedo.g, albedo.b, 1));
                    graph->register_node(value_node);
                    value_node->get_output(0)->link(graph->get_base_color());
                }

                if (AI_SUCCESS == native_material->GetTexture(aiTextureType_NORMALS, 0, &path))
                {
					auto native_path = directory / std::string(path.C_Str());
					auto diff = get_texture(native_path);
                    auto tex_node = std::make_shared<TextureNode>(diff);
                    graph->register_node(tex_node);
                    graph->get_texcoord()->link(tex_node->get_input(0));
                    tex_node->get_output(0)->link(graph->get_normals());
                }

                else if (AI_SUCCESS == native_material->GetTexture(aiTextureType_HEIGHT, 0, &path))
                {
					auto native_path = directory / std::string(path.C_Str());
					auto diff = get_texture(native_path);
                    auto tex_node = std::make_shared<TextureNode>(diff);
                    graph->register_node(tex_node);
                    graph->get_texcoord()->link(tex_node->get_input(0));
                    tex_node->get_output(0)->link(graph->get_normals());
                }

          /*      if (AI_SUCCESS == native_material->GetTexture(aiTextureType_SPECULAR, 0, &path))
                {
                    std::string native_path = directory + std::string(path.C_Str());
                    auto diff = get_texture(native_path);
                    auto tex_node = std::make_shared<TextureNode>(diff);
                    graph->register_node(tex_node);
                    graph->get_texcoord()->link(tex_node->get_input(0));
                    tex_node->get_output(0)->link(graph->get_roughness());
                }

                else*/
                {
					float albedo;

					if (AI_SUCCESS == native_material->Get(AI_MATKEY_SHININESS, albedo))
					{
						albedo /= 255;

						if (albedo > 1)
							albedo = 1;

						auto value_node = std::make_shared<ScalarNode>(albedo);
						graph->register_node(value_node);
						value_node->get_output(0)->link(graph->get_roughness());
					}
                }
				
     /*           if (AI_SUCCESS == native_material->GetTexture(aiTextureType_SHININESS, 0, &path))
                {
                    std::string native_path = directory + std::string(path.C_Str());
                    auto diff = get_texture(native_path);
                    auto tex_node = std::make_shared<TextureNode>(diff);
                    graph->register_node(tex_node);
                    graph->get_texcoord()->link(tex_node->get_input(0));
                    tex_node->get_output(0)->link(graph->get_mettalic_color());
                }

                else*/       
				
				
				


				{
                    float albedo;

                    if (AI_SUCCESS == native_material->Get(AI_MATKEY_REFLECTIVITY, albedo))
                    {
                        albedo /= 255;

                        if (albedo > 1)
                            albedo = 1;

                        auto value_node = std::make_shared<ScalarNode>(albedo);
                        graph->register_node(value_node);
                        value_node->get_output(0)->link(graph->get_mettalic());
                    }

                    else
                    {
       //                 if (tex_node)tex_node->get_output(0)->link(graph->get_mettalic_color());
                    }
                }

                //  m->shader = Render::pixel_shader::get_resource({ "material.hlsl", "PS", 0, {} });
                //  m->shader_color = Render::pixel_shader::get_resource({ "material.hlsl", "PS_Color", 0, {} });
                //   m->compile();
                aiString mat_name;
                //if (aiGetMaterialString(native_material, AI_MATKEY_NAME, &mat_name) == aiReturn_SUCCESS)
                //    m->name.assign(mat_name.C_Str(), mat_name.C_Str() + mat_name.length);
                //MaterialAsset* mat = m;//(new MaterialAsset(m));

				materials::universal_material* m = (new materials::universal_material(graph));

                m->register_new();
				materials[i] = m->get_ptr<MaterialAsset>();
                return true;
            }));
        }

		for (auto && e : tasks)
			e.wait();

		tasks.clear();
        result->meshes.resize(scene->mNumMeshes);

        for (unsigned int i = 0; i < scene->mNumMeshes; i++)
        {
            auto& native_mesh = scene->mMeshes[i];
            tasks.emplace_back(thread_pool::get().enqueue([&vertices, &indices, native_mesh, vertex_count, index_count, result, i, scale,&materials]()
            {
                auto position_function = [native_mesh, scale](unsigned int index)->vec3
                {
                    return vec3(native_mesh->mVertices[index].x * scale, native_mesh->mVertices[index].y * scale, native_mesh->mVertices[index].z * scale);
                };
                auto tangent_function = [native_mesh](unsigned int index)->vec3
                {
                    return vec3(native_mesh->mTangents[index].x, native_mesh->mTangents[index].y, native_mesh->mTangents[index].z);
                };
                auto bitangent_function = [native_mesh](unsigned int index)->vec3
                {
                    return vec3(native_mesh->mBitangents[index].x, native_mesh->mBitangents[index].y, native_mesh->mBitangents[index].z);
                };
                auto normals_function = [native_mesh](unsigned int index)->vec3
                {
                    return vec3(native_mesh->mNormals[index].x, native_mesh->mNormals[index].y, native_mesh->mNormals[index].z);
                };
                auto tc_function = [native_mesh](unsigned int index)->vec2
                {
                    if (!native_mesh->HasTextureCoords(0))
                        return vec2(0, 0);


                    return vec2(native_mesh->mTextureCoords[0][index].x, native_mesh->mTextureCoords[0][index].y);
                };
                auto faces_function = [native_mesh](unsigned int index)->ivec3
                {
                    return ivec3(native_mesh->mFaces[index].mIndices[0], native_mesh->mFaces[index].mIndices[1], native_mesh->mFaces[index].mIndices[2]);
                };

                for (unsigned int i = 0; i < native_mesh->mNumVertices; i++)
                    vertices[vertex_count + i].pos = position_function(i);

                if (native_mesh->HasNormals() && native_mesh->HasTextureCoords(0))
                {
                   // tangent_generator tan_gen(position_function, tc_function, normals_function, faces_function, native_mesh->mNumFaces, native_mesh->mNumVertices);

                    for (unsigned int i = 0; i < native_mesh->mNumVertices; i++)
                    {
                        vertices[vertex_count + i].normal = normals_function(i);


                         auto tan1 = tangent_function(i);
                         auto tan2 = bitangent_function(i);
                         auto norm = normals_function(i);
                        vertices[vertex_count + i].tangent =vec4(tan1, (vec3::dot(vec3::cross(norm, tan1), tan2) < 0.0f) ? -1.0f : 1.0f);
                    }
				}
				else if(native_mesh->HasNormals())
				{
					for (unsigned int i = 0; i < native_mesh->mNumVertices; i++)
						vertices[vertex_count + i].normal = normals_function(i);
				}

                if (native_mesh->HasTextureCoords(0))
                    for (unsigned int i = 0; i < native_mesh->mNumVertices; i++)
                        vertices[vertex_count + i].tc = tc_function(i);

                for (unsigned int f = 0; f < native_mesh->mNumFaces; f++)
                {
                    auto& face = native_mesh->mFaces[f];
                    indices[index_count + f * 3] = 0*vertex_count + (face.mIndices[0]);
                    indices[index_count + f * 3 + 1] = 0*vertex_count + (face.mIndices[1]);
                    indices[index_count + f * 3 + 2] = 0*vertex_count + (face.mIndices[2]);
                }


				result->materials[i] = materials[native_mesh->mMaterialIndex];
                MeshInfo mesh;
				mesh.material = i;// native_mesh->mMaterialIndex;
                mesh.vertex_count = native_mesh->mNumVertices;
                mesh.vertex_offset = vertex_count;
                mesh.index_count = native_mesh->mNumFaces * 3;
                mesh.index_offset = index_count;
                mesh.primitive = get_best_primitive(position_function, native_mesh->mNumVertices);
             

                /// hate this, just do normally later
                std::vector<float3> poses;

                for(UINT i=0;i< mesh.vertex_count;i++)
                {
                    poses.push_back(vertices[i+ mesh.vertex_offset].pos);//argh!!
                }
                Meshletize(64, 126,
                    indices.data() + mesh.index_offset, mesh.index_count,
                    poses.data(), mesh.vertex_count,
                    mesh.meshlets
                );

                for (auto& m : mesh.meshlets)
                {
                    ComputeCullData(poses.data(), mesh.vertex_count, m, 0);

                }
               
                result->meshes[i] = (mesh);

                return true;
            }));
            index_count += native_mesh->mNumFaces * 3;
            vertex_count += native_mesh->mNumVertices;


            //   result_mesh->material_index = native_mesh->mMaterialIndex;
        }

        for (auto && e : tasks)
            e.wait();

        result->vertex_buffer = std::move(vertices);
		result->index_buffer = std::move(indices);

        std::function<void(aiNode*, MeshNode& obj, MeshNode*)>  node_parser = [&](aiNode * node, MeshNode & obj, MeshNode * parent)
        {
            if (!node) return;

            //obj.local_matrix.identity();
			//obj.mesh_matrix.identity();

            memcpy(&obj.local_matrix, node->mTransformation[0], sizeof(obj.local_matrix));
            obj.local_matrix.transpose();

            if (parent)
                obj.mesh_matrix = obj.local_matrix * parent->mesh_matrix;
            else
                obj.mesh_matrix = obj.local_matrix;
                
       
            for (unsigned int i = 0; i < node->mNumMeshes; i++)
            {

                obj.childs.emplace_back();
                obj.childs.back().mesh_id=node->mMeshes[i];
                obj.childs.back().local_matrix.identity();
				obj.childs.back().mesh_matrix.identity();

            }
                
            for (unsigned int i = 0; i < node->mNumChildren; i++)
            {
                obj.childs.emplace_back();
                node_parser(node->mChildren[i], obj.childs.back(), &obj);
            }
        };
        node_parser(scene->mRootNode, result->root_node, nullptr);
        result->calculate_size();








        return result;
    }

    return nullptr;
}