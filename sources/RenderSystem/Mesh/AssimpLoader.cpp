module;
// Assimp macros don't propagate through the named module boundary — redefine them here.
#define AI_SUCCESS              aiReturn_SUCCESS
#define AI_MATKEY_COLOR_DIFFUSE "$clr.diffuse", 0, 0
#define AI_MATKEY_SHININESS     "$mat.shininess", 0, 0
#define AI_MATKEY_REFLECTIVITY  "$mat.reflectivity", 0, 0
// material.h defines aiGetMaterialFloat as static inline; static functions from
// exported headers don't cross the module boundary into this TU, so include it
// explicitly in the global module fragment to keep the definition reachable.
#include <assimp/material.h>
// config.h is macro-only (importer property name strings), so it also has to be
// included here rather than relied on through `import assimp`.
#include <assimp/config.h>

module Graphics;
import Core;
import HAL;
import GUI;
import Graphics;
import assimp;

#include "Simplifyer/Simplifyer.h"
#include "Mesh/MeshletGeneration.h"

struct MeshLoadingSettings
{
	float scale = 1;
	bool materials_remove = true;
	std::map<std::filesystem::path, AssetStorage::ptr> load_textures;

};
class LoadingWindow : public GUI::Elements::window
{
	MeshLoadingSettings settings;

	GUI::Elements::label::ptr scale_value_label;

	void update_scale_label()
	{
		char buf[16];
		snprintf(buf, sizeof(buf), "%.3f", settings.scale);
		scale_value_label->text = buf;
	}

public:
	std::promise<MeshLoadingSettings> result;

	LoadingWindow(std::string_view name, MeshLoadingSettings& _settings) : settings(_settings)
	{
		auto short_name = std::filesystem::path(name).filename().string();
		set_title("Import: " + short_name);

		docking     = GUI::dock::FILL;
		width_size  = GUI::size_type::FIXED;
		height_size = GUI::size_type::FIXED;
		size        = {420, 200};

		contents->width_size  = GUI::size_type::MATCH_PARENT;
		contents->height_size = GUI::size_type::MATCH_PARENT;
		contents->padding     = {8, 8, 8, 8};

		// ---- Import Settings section ----
		auto import_section = std::make_shared<GUI::Elements::collapsible_section>("Import Settings");
		add_child(import_section);

		// Scale row: label | slider | value
		auto scale_row = std::make_shared<GUI::base>();
		scale_row->docking     = GUI::dock::TOP;
		scale_row->height_size = GUI::size_type::FIXED;
		scale_row->size        = {0, 22};
		import_section->content->add_child(scale_row);

		auto scale_lbl = std::make_shared<GUI::Elements::label>();
		scale_lbl->text        = "Scale";
		scale_lbl->docking     = GUI::dock::LEFT;
		scale_lbl->width_size  = GUI::size_type::FIXED;
		scale_lbl->size        = {60, 0};
		scale_lbl->magnet_text = FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP;
		scale_row->add_child(scale_lbl);

		scale_value_label = std::make_shared<GUI::Elements::label>();
		scale_value_label->docking     = GUI::dock::RIGHT;
		scale_value_label->width_size  = GUI::size_type::FIXED;
		scale_value_label->size        = {44, 0};
		scale_value_label->magnet_text = FW1_RIGHT | FW1_VCENTER | FW1_NOWORDWRAP;
		scale_row->add_child(scale_value_label);

		auto scale_slider = std::make_shared<GUI::Elements::float_slider>();
		scale_slider->min        = 0.01f;
		scale_slider->max        = 100.0f;
		scale_slider->value      = settings.scale;
		scale_slider->docking    = GUI::dock::FILL;
		scale_slider->height_size = GUI::size_type::FIXED;
		scale_slider->size       = {0, 14};
		scale_slider->y_type     = GUI::pos_y_type::CENTER;
		scale_row->add_child(scale_slider);

		update_scale_label();
		scale_slider->on_change = [this](float v)
		{
			settings.scale = v;
			update_scale_label();
		};

		import_section->content->add_child(std::make_shared<GUI::Elements::separator>());

		// Remove duplicate materials toggle
		auto mats_row = std::make_shared<GUI::Elements::check_box_text>();
		mats_row->docking = GUI::dock::TOP;
		mats_row->x_type  = GUI::pos_x_type::LEFT;
		mats_row->get_label()->text = "Remove duplicate materials";
		mats_row->get_check()->size = {30, 16};
		mats_row->get_check()->set_checked(settings.materials_remove);
		mats_row->on_check = [this](bool v) { settings.materials_remove = v; };
		import_section->content->add_child(mats_row);

		// ---- Textures section ----
		if (!settings.load_textures.empty())
		{
			add_child(std::make_shared<GUI::Elements::separator>());

			auto tex_section = std::make_shared<GUI::Elements::collapsible_section>("Textures");
			add_child(tex_section);

			auto list = std::make_shared<GUI::Elements::scroll_container>();
			list->docking     = GUI::dock::FILL;
			list->height_size = GUI::size_type::FIXED;
            list->size={160,160};
			tex_section->content->add_child(list);

			for (auto& p : settings.load_textures)
			{
				if (!p.second) continue;

				auto item = std::make_shared<GUI::Elements::check_box_text>();
				item->docking = GUI::dock::TOP;
				item->x_type  = GUI::pos_x_type::LEFT;
				item->get_label()->text = p.first.filename().generic_string();
				item->get_check()->set_checked(true);

				auto second_val = p.second;
				auto first_val  = p.first;
				item->on_check = [this, first_val, second_val](bool v)
				{
					settings.load_textures[first_val] = v ? second_val : nullptr;
				};
				list->add_child(item);
			}
		}

		// ---- OK button ----
		add_child(std::make_shared<GUI::Elements::separator>());

		auto ok = std::make_shared<GUI::Elements::button>();
		ok->docking     = GUI::dock::TOP;
		ok->width_size  = GUI::size_type::MATCH_PARENT;
		ok->height_size = GUI::size_type::FIXED;
		ok->size        = {0, 30};
		ok->margin      = {0, 6, 0, 0};
		ok->get_label()->text = "Import";
		ok->on_click = [this](GUI::Elements::button::ptr)
		{
			result.set_value(settings);
			remove_from_parent();
		};
		add_child(ok);
	}
};
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
            auto file = FileSystem::get().get_file(to_path(filename));

            if (file)
                data = file->load_all();
        }
    public:

        size_t Read(void* pvBuffer, size_t pSize, size_t pCount)
        {
            // fread contract: return the number of complete items read, not bytes
            // (the glTF importers check `Read(&header, sizeof(header), 1) != 1`).
            if (!pSize || pos >= data.size())
                return 0;

            size_t items = std::min((data.size() - pos) / pSize, pCount);
            std::memcpy(pvBuffer, data.data() + pos, items * pSize);
            pos += items * pSize;
            return items;
        }


        size_t Write(const void* pvBuffer, size_t pSize, size_t pCount) { return pCount; }
        aiReturn Seek(size_t pOffset, aiOrigin pOrigin)
        {
            if (pOrigin == aiOrigin::aiOrigin_CUR)
                pos += pOffset;

            if (pOrigin == aiOrigin::aiOrigin_END)
                pos = data.size() + pOffset; // fseek semantics: offset relative to end

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
            return !!FileSystem::get().get_file(path / to_path(pFile));
        }
        // Get the path delimiter character we'd like to see
        virtual char getOsSeparator() const override
        {
            return '/';
        }
        // ... and finally a method to open a custom stream
        virtual Assimp::IOStream* Open(const char* pFile, const char* pMode = "rb") override
        {
			auto new_path = path / to_path(pFile);
			// weakly_canonical: canonical() throws for nonexistent paths, and Assimp
			// probes embedded-texture names (e.g. "Image_9") as if they were files.
			new_path = std::filesystem::weakly_canonical(new_path);
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


// FBX (and some OBJ/DAE) material paths are frequently baked as the exporting
// artist's absolute machine path. std::filesystem::path::operator/ discards the
// base directory whenever the right-hand side is already absolute, so joining
// directory / path silently reproduces that dead path instead of resolving next
// to the model. Fall back to the texture's bare filename next to the model file.
static std::filesystem::path resolve_texture_path(const std::filesystem::path& directory, const char* raw_path)
{
	auto path = to_path(raw_path);

	if (path.is_absolute())
		return directory / path.filename();

	return directory / path;
}

// Embedded textures (FBX/glTF pack them inside the model file) come as either a
// compressed image blob (mHeight == 0, achFormatHint = extension) or a raw aiTexel
// array (b,g,r,a byte order == B8G8R8A8).
static HAL::texture_data::ptr texture_data_from_embedded(const aiTexture* tex)
{
	if (tex->mHeight == 0)
		return HAL::texture_data::load_from_memory(tex->pcData, tex->mWidth, tex->achFormatHint, HAL::texture_data::GENERATE_MIPS);

	auto data = std::make_shared<HAL::texture_data>(1, 1, tex->mWidth, tex->mHeight, 1, HAL::Format::B8G8R8A8_UNORM);
	auto& mip = data->array[0]->mips[0];
	std::memcpy(mip->data.data(), tex->pcData, mip->data.size());
	return data;
}

std::shared_ptr<MeshData> MeshData::load_assimp(const std::string& file_name, resource_file_depender& files, AssetLoadingContext::ptr & context)
{
    Assimp::Importer importer;
	std::filesystem::path directory = to_path(file_name).parent_path();

 //   if (directory.size())
   //     directory += std::string("\\");

	// TODO: need delete?
    importer.SetIOHandler(new MyIOSystem(directory, files));
	importer.SetProgressHandler(new MyProgressHandler(context));

	// The mesh loop assumes 3 indices per face; SortByPType with this property
	// strips point/line primitives (including degenerates demoted by FindDegenerates).
	importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);

    //  auto file = FileSystem::get().get_file(file_name);
    const aiScene* scene;

    try
    {
        scene = importer.ReadFile(to_path(file_name).filename().generic_string(),
            aiProcess_JoinIdenticalVertices |
            aiProcess_GenBoundingBoxes |
            aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_SortByPType |
            aiProcess_FindDegenerates |
            aiProcess_FindInvalidData |
            aiProcess_GenUVCoords |
            aiProcess_TransformUVCoords |
            aiProcess_FlipUVs |
            aiProcess_GenSmoothNormals/* | aiProcess_PreTransformVertices*/);
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

		// keyed the same way as load_textures, so material creation finds them by path
		std::map<std::filesystem::path, const aiTexture*> embedded_textures;

		auto check_texture = [&load_textures](std::filesystem::path name)
		{
			auto it = load_textures.find(name);

			if (it == load_textures.end())
			{
				auto file_name = name.filename();

				load_textures[name] = AssetManager::get().find_storage_by_name(file_name.generic_wstring());



			}
		};

		auto check_assimp_texture = [&directory, &check_texture, &load_textures, &embedded_textures, &scene](aiMaterial*native_material, aiTextureType type) {
			aiString path;
			if (AI_SUCCESS == native_material->GetTexture(type, 0, &path))
			{
				auto native_path = resolve_texture_path(directory, path.C_Str());

				if (auto embedded = scene->GetEmbeddedTexture(path.C_Str()))
				{
					embedded_textures[native_path] = embedded;
					// Embedded names ("*0", "Image_0") are file-local — never reuse a
					// registered asset by that name (a previous import's "*2" can be a
					// completely different image); always decode from this file's data.
					load_textures.emplace(native_path, nullptr);
				}
				else
					check_texture(native_path);
			}

		};


		for (unsigned int i = 0; i < scene->mNumMaterials; i++)
		{
			auto& native_material = scene->mMaterials[i];

			check_assimp_texture(native_material, aiTextureType_DIFFUSE);
			check_assimp_texture(native_material, aiTextureType_BASE_COLOR);
			check_assimp_texture(native_material, aiTextureType_NORMALS);
			check_assimp_texture(native_material, aiTextureType_HEIGHT);
			check_assimp_texture(native_material, aiTextureType_METALNESS);
			check_assimp_texture(native_material, aiTextureType_DIFFUSE_ROUGHNESS);
			check_assimp_texture(native_material, aiTextureType_EMISSION_COLOR);
			check_assimp_texture(native_material, aiTextureType_EMISSIVE);

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

				tasks.emplace_back(thread_pool::get().enqueue([t, index,&load_textures,&embedded_textures]() {

					TextureAsset* tex;
					auto embedded = embedded_textures.find(index);

					if (embedded != embedded_textures.end())
					{
						auto* ai_tex = embedded->second;
						// glTF refers to embedded textures as "*0", "*1", ...; prefer the
						// image's own name (e.g. "Image_0") for the registered asset.
						auto name = ai_tex->mFilename.length
							? to_path(ai_tex->mFilename.C_Str()).filename().wstring()
							: index.filename().wstring();
						tex = new TextureAsset(texture_data_from_embedded(ai_tex), name);
					}
					else
						tex = new TextureAsset(t.first);

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

        std::vector<Table::mesh_vertex_input::Compiled> vertices(vertex_count);
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

                if (AI_SUCCESS == native_material->GetTexture(aiTextureType_DIFFUSE, 0, &path)
                 || AI_SUCCESS == native_material->GetTexture(aiTextureType_BASE_COLOR, 0, &path))
                {
                   auto native_path = resolve_texture_path(directory, path.C_Str());
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
					auto native_path = resolve_texture_path(directory, path.C_Str());
					auto diff = get_texture(native_path);
                    auto tex_node = std::make_shared<TextureNode>(diff);
                    graph->register_node(tex_node);
                    graph->get_texcoord()->link(tex_node->get_input(0));
                    tex_node->get_output(0)->link(graph->get_normals());
                }

                else if (AI_SUCCESS == native_material->GetTexture(aiTextureType_HEIGHT, 0, &path))
                {
					auto native_path = resolve_texture_path(directory, path.C_Str());
					auto diff = get_texture(native_path);
                    auto tex_node = std::make_shared<TextureNode>(diff);
                    graph->register_node(tex_node);
                    graph->get_texcoord()->link(tex_node->get_input(0));
                    tex_node->get_output(0)->link(graph->get_normals());
                }

                // glTF packs metallic-roughness into one texture: G = roughness, B = metallic.
                // Standalone (grayscale) maps are sampled from R instead.
                aiString metal_path, rough_path;
                bool has_metal_tex = AI_SUCCESS == native_material->GetTexture(aiTextureType_METALNESS, 0, &metal_path);
                bool has_rough_tex = AI_SUCCESS == native_material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &rough_path);
                bool packed = has_metal_tex && has_rough_tex && std::strcmp(metal_path.C_Str(), rough_path.C_Str()) == 0;

                TextureNode::ptr metal_rough_node;

                if (has_rough_tex)
                {
                    metal_rough_node = std::make_shared<TextureNode>(get_texture(resolve_texture_path(directory, rough_path.C_Str())));
                    graph->register_node(metal_rough_node);
                    graph->get_texcoord()->link(metal_rough_node->get_input(0));
                    metal_rough_node->get_output(packed ? 2 : 1)->link(graph->get_roughness());
                }

                else
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
				
                if (has_metal_tex)
                {
                    auto node = metal_rough_node;

                    if (!packed)
                    {
                        node = std::make_shared<TextureNode>(get_texture(resolve_texture_path(directory, metal_path.C_Str())));
                        graph->register_node(node);
                        graph->get_texcoord()->link(node->get_input(0));
                    }

                    node->get_output(packed ? 3 : 1)->link(graph->get_mettalic());
                }

                else
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
                }

                if (AI_SUCCESS == native_material->GetTexture(aiTextureType_EMISSION_COLOR, 0, &path)
                 || AI_SUCCESS == native_material->GetTexture(aiTextureType_EMISSIVE, 0, &path))
                {
                    auto node = std::make_shared<TextureNode>(get_texture(resolve_texture_path(directory, path.C_Str())), true);
                    graph->register_node(node);
                    graph->get_texcoord()->link(node->get_input(0));
                    node->get_output(0)->link(graph->get_glow());
                }

                //  m->shader = HAL::pixel_shader::get_resource({ "material.hlsl", "PS", 0, {} });
                //  m->shader_color = HAL::pixel_shader::get_resource({ "material.hlsl", "PS_Color", 0, {} });
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
                    indices[index_count + f * 3] = face.mIndices[0];
                    indices[index_count + f * 3 + 1] = face.mIndices[1];
                    indices[index_count + f * 3 + 2] = face.mIndices[2];
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

            std::memcpy(&obj.local_matrix, node->mTransformation[0], sizeof(obj.local_matrix));
            obj.local_matrix.transpose();

            // Vertex positions are pre-scaled by `scale` (position_function below), but
            // node translations come straight from the FBX file's native units. Scale just
            // the translation row to match - the rotation/local-scale block must stay as
            // authored, or the hierarchy's accumulated rotations would be corrupted.
            obj.local_matrix.a41 *= scale;
            obj.local_matrix.a42 *= scale;
            obj.local_matrix.a43 *= scale;

            if (parent)
                obj.mesh_matrix = obj.local_matrix * parent->mesh_matrix;
            else
                obj.mesh_matrix = obj.local_matrix;
                
       
            for (unsigned int i = 0; i < node->mNumMeshes; i++)
            {

                obj.childs.emplace_back();
                obj.childs.back().mesh_id=node->mMeshes[i];
                obj.childs.back().local_matrix.identity();
				obj.childs.back().mesh_matrix = obj.mesh_matrix;

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