module;
// Assimp macros don't propagate through the named module boundary — redefine them here.
#define AI_SUCCESS              aiReturn_SUCCESS
#define AI_MATKEY_COLOR_DIFFUSE "$clr.diffuse", 0, 0
#define AI_MATKEY_SHININESS     "$mat.shininess", 0, 0
#define AI_MATKEY_REFLECTIVITY  "$mat.reflectivity", 0, 0
#define AI_MATKEY_OPACITY       "$mat.opacity", 0, 0
#define AI_MATKEY_EMISSIVE_INTENSITY "$mat.emissiveIntensity", 0, 0
#define AI_MATKEY_COLOR_SPECULAR "$clr.specular", 0, 0
#define AI_MATKEY_GLOSSINESS_FACTOR "$mat.glossinessFactor", 0, 0
// material.h defines aiGetMaterialFloat as static inline; static functions from
// exported headers don't cross the module boundary into this TU, so include it
// explicitly in the global module fragment to keep the definition reachable.
#include <assimp/material.h>
#ifndef AI_MATKEY_REFRACTI
#define AI_MATKEY_REFRACTI "$mat.refracti", 0, 0
#endif
#ifndef AI_MATKEY_TRANSMISSION_FACTOR
#define AI_MATKEY_TRANSMISSION_FACTOR "$mat.transmission.factor", 0, 0
#endif
#ifndef AI_MATKEY_VOLUME_THICKNESS_FACTOR
#define AI_MATKEY_VOLUME_THICKNESS_FACTOR "$mat.volume.thicknessFactor", 0, 0
#endif
#ifndef AI_MATKEY_VOLUME_ATTENUATION_DISTANCE
#define AI_MATKEY_VOLUME_ATTENUATION_DISTANCE "$mat.volume.attenuationDistance", 0, 0
#endif
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

// BC5/R8G8 are the standard 2-channel encodings for a tangent-space normal
// map (X/Y only -- Z is always the larger, positive, reconstructible
// component, so it's dropped to save space). A texture in one of these
// formats linked as a normal map needs ReconstructNormalZNode instead of a
// raw sample, since there's no blue channel to read.
static bool is_two_channel_normal_format(HAL::Format format)
{
    return format == HAL::Format::BC5_UNORM  || format == HAL::Format::BC5_SNORM
        || format == HAL::Format::R8G8_UNORM || format == HAL::Format::R8G8_SNORM;
}

// A texture stored with only 1 or 2 channels can't hold a real RGB tint --
// whatever the tool that authored it meant, it's a single scalar's worth of
// information (a factor/intensity/mask), not a color. Sampling it as .rgb
// reads whichever channels exist and hardware-zeroes the rest, which for a
// property like specular easily reads as a nonsense colored tint (e.g. a
// 2-channel texture with its real value in G alone reads back pure green).
static bool is_scalar_mask_format(HAL::Format format)
{
    return format == HAL::Format::R8_UNORM    || format == HAL::Format::R8_SNORM
        || format == HAL::Format::R16_UNORM   || format == HAL::Format::R16_SNORM
        || format == HAL::Format::BC4_UNORM   || format == HAL::Format::BC4_SNORM
        || is_two_channel_normal_format(format);
}

// Most GPU formats are nominally 4-component even for RGB-only content (a
// plain "RGB, no alpha" DXGI format barely exists), so a blanket "does this
// format have an alpha channel at all" check would fire for nearly every
// texture and needlessly flip otherwise-fully-opaque materials into the
// transparent render path (see get_opacity()'s own "only wire when != 1"
// comment below for why that's worth avoiding). Restrict to formats that are
// specifically *chosen* because their alpha carries real data -- BC1 in
// particular is the conventional "opaque color, don't bother with alpha"
// compressed format, so it's deliberately excluded here.
static bool format_likely_has_real_alpha(HAL::Format format)
{
    return format == HAL::Format::BC2_UNORM || format == HAL::Format::BC2_UNORM_SRGB
        || format == HAL::Format::BC3_UNORM || format == HAL::Format::BC3_UNORM_SRGB
        || format == HAL::Format::BC7_UNORM || format == HAL::Format::BC7_UNORM_SRGB
        || format == HAL::Format::R8G8B8A8_UNORM || format == HAL::Format::R8G8B8A8_UNORM_SRGB
        || format == HAL::Format::B8G8R8A8_UNORM || format == HAL::Format::B8G8R8A8_UNORM_SRGB
        || format == HAL::Format::R16G16B16A16_UNORM || format == HAL::Format::R16G16B16A16_FLOAT
        || format == HAL::Format::R32G32B32A32_FLOAT;
}

// link() returns false rather than throwing on an incompatible/already-
// connected graph parameter (e.g. a node registering an output with no
// explicit type -- defaults to strict_parameter -- linked into a strictly
// typed MaterialGraph slot like base_color/metallic; see SpecToMetNode's own
// fix for a concrete case of this). A material with a link that silently
// failed imports "successfully" but with a slot quietly left unset, so
// material-graph wiring in this file throws instead of ignoring the result.
static void link_or_throw(const FlowGraph::parameter::ptr& from, const FlowGraph::parameter::ptr& to, const char* description)
{
    if (!from->link(to))
        throw std::runtime_error(std::string("AssimpLoader: failed to link ") + description + " -- incompatible or already-connected graph parameter");
}

// Different scenes pack completely different data into a legacy FBX
// "Specular" texture slot -- some really are a spec/gloss RGB tint, others
// (e.g. Amazon Lumberyard Bistro: R=Occlusion, G=Roughness, B=Metalness) are
// a packed ORM texture filed there instead of METALNESS/DIFFUSE_ROUGHNESS.
// There's no reliable way to auto-detect which convention a given asset
// uses, so it's a per-import choice (see LoadingWindow's "Specular" section).
enum class SpecularChannelUse { Ignore, SpecularColor, Metalness, Roughness };

static const char* to_string(SpecularChannelUse v)
{
	switch (v)
	{
	case SpecularChannelUse::Ignore:        return "Ignore";
	case SpecularChannelUse::SpecularColor: return "Specular Color";
	case SpecularChannelUse::Metalness:     return "Metalness";
	case SpecularChannelUse::Roughness:     return "Roughness";
	}
	return "Ignore";
}

struct MeshLoadingSettings
{
	float scale = 1;
	bool materials_remove = true;
	bool load_specular_textures = true;
	SpecularChannelUse specular_r = SpecularChannelUse::SpecularColor;
	SpecularChannelUse specular_g = SpecularChannelUse::SpecularColor;
	SpecularChannelUse specular_b = SpecularChannelUse::SpecularColor;
	std::map<std::filesystem::path, AssetStorage::ptr> load_textures;
	// Set during the pre-scan (before LoadingWindow is constructed) -- gates
	// whether the Specular UI section shows at all.
	bool any_specular_source = false;

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
		size        = {420, 400};

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

		// ---- Specular section ----
		// Only shown when the scene actually has a specular texture/color
		// somewhere -- see SpecularChannelUse's own comment for why this
		// needs to be a per-import choice rather than something auto-detected.
		if (settings.any_specular_source)
		{
			add_child(std::make_shared<GUI::Elements::separator>());

			auto spec_section = std::make_shared<GUI::Elements::collapsible_section>("Specular");
			add_child(spec_section);

			auto load_row = std::make_shared<GUI::Elements::check_box_text>();
			load_row->docking = GUI::dock::TOP;
			load_row->x_type  = GUI::pos_x_type::LEFT;
			load_row->get_label()->text = "Load specular textures";
			load_row->get_check()->size = {30, 16};
			load_row->get_check()->set_checked(settings.load_specular_textures);
			load_row->on_check = [this](bool v) { settings.load_specular_textures = v; };
			spec_section->content->add_child(load_row);

			auto add_channel_row = [this, &spec_section](std::string_view label_text, SpecularChannelUse& field)
			{
				auto row = std::make_shared<GUI::Elements::layouts::horizontal>();
				row->docking = GUI::dock::TOP;
				row->x_type  = GUI::pos_x_type::LEFT;

				auto label = std::make_shared<GUI::Elements::label>();
				label->text = std::string(label_text);
				row->add_child(label);

				static const std::pair<const char*, SpecularChannelUse> options[] = {
					{"Ignore",         SpecularChannelUse::Ignore},
					{"Specular Color", SpecularChannelUse::SpecularColor},
					{"Metalness",      SpecularChannelUse::Metalness},
					{"Roughness",      SpecularChannelUse::Roughness},
				};

				auto combo = std::make_shared<GUI::Elements::combo_box>();
				combo->size = {120, combo->size->y};
				for (auto& opt : options)
				{
					auto item = combo->add_item(opt.first);
					auto value = opt.second;
					auto combo_wptr = GUI::Elements::combo_box::wptr(combo);
					item->on_select = [this, &field, value, combo_wptr]()
					{
						field = value;
						if (auto c = combo_wptr.lock())
							c->get_label()->text = to_string(value);
					};
				}
				combo->get_label()->text = to_string(field);
				row->add_child(combo);

				spec_section->content->add_child(row);
			};

			add_channel_row("Specular R", settings.specular_r);
			add_channel_row("Specular G", settings.specular_g);
			add_channel_row("Specular B", settings.specular_b);
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
			check_assimp_texture(native_material, aiTextureType_OPACITY);
			check_assimp_texture(native_material, aiTextureType_SPECULAR);

			aiString spec_path_probe;
			aiColor3D spec_color_probe;
			if (AI_SUCCESS == native_material->GetTexture(aiTextureType_SPECULAR, 0, &spec_path_probe)
			 || AI_SUCCESS == native_material->Get(AI_MATKEY_COLOR_SPECULAR, spec_color_probe))
				settings.any_specular_source = true;
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

        std::vector<Table::Meshes::mesh_vertex_input::Compiled> vertices(vertex_count);
        std::vector<UINT32> indices(index_count);
        vertex_count = 0;
        index_count = 0;
        auto get_texture = [&load_textures, &m](std::filesystem::path name)->TextureAsset::ptr
        {
            // Any of these can legitimately be null: a texture type referenced
            // by a material but never passed to check_assimp_texture() above
            // leaves `name` out of load_textures entirely (operator[] would
            // silently default-construct a null entry rather than report
            // that); find_storage_by_name() (populating load_textures) returns
            // null when no asset matches that filename at all; and
            // AssetStorage::get_asset() returns null when the backing file
            // exists as a registered asset but fails to actually load. Fall
            // back to the engine's own missing-texture placeholder rather than
            // crashing on a null dereference either way, so one bad/absent
            // texture reference doesn't fail the whole mesh import.
            auto it = load_textures.find(name);
            auto storage = (it != load_textures.end()) ? it->second : nullptr;
            auto asset = storage ? storage->get_asset() : nullptr;
            auto tex = asset ? asset->get_ptr<TextureAsset>() : nullptr;

            if (!tex)
            {
                const char* reason = it == load_textures.end() ? "never registered (missing check_assimp_texture call for its texture type?)"
                                    : !storage             ? "no matching asset found for this filename"
                                    : !asset                ? "asset storage found but failed to load"
                                    :                          "asset found but isn't a TextureAsset";

                Log::get() << Log::LEVEL_WARNING << "AssimpLoader: texture \"" << name.string()
                    << "\" could not be loaded (" << reason << ") -- using missing_texture placeholder" << Log::endl;

                tex = EngineAssets::missing_texture.get_asset();
            }

            return tex;
        };
        std::vector<std::future<bool>> tasks;

		std::vector<MaterialAsset::ptr> materials;

		materials.resize(scene->mNumMaterials);

        result->materials.resize(scene->mNumMeshes);

        for (unsigned int i = 0; i < scene->mNumMaterials; i++)
        {
            bool load_specular_textures = settings.load_specular_textures;
            SpecularChannelUse specular_r = settings.specular_r;
            SpecularChannelUse specular_g = settings.specular_g;
            SpecularChannelUse specular_b = settings.specular_b;

            tasks.emplace_back(thread_pool::get().enqueue([&materials ,scene,i,directory, &get_texture,
                load_specular_textures, specular_r, specular_g, specular_b]()
            {
              try
              {
                auto& native_material = scene->mMaterials[i];
                aiString path;
                MaterialGraph::ptr graph(new MaterialGraph);
                SamplingNode::ptr  tex_node;
                TextureAsset::ptr  albedo_tex;

                // glTF-style metallic-roughness texture presence (used below to
                // decide the workflow) and the PBR Specular/Glossiness workflow
                // (3ds Max Phong/Blinn, Substance Painter spec/gloss exports,
                // glTF's KHR_materials_pbrSpecularGlossiness) -- decided up front
                // since base_color/metallic are single-input graph outputs and
                // must be linked exactly once, whichever workflow supplies them.
                aiString metal_path, rough_path, spec_path;
                bool has_metal_tex = AI_SUCCESS == native_material->GetTexture(aiTextureType_METALNESS, 0, &metal_path);
                bool has_rough_tex = AI_SUCCESS == native_material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &rough_path);
                bool packed = has_metal_tex && has_rough_tex && std::strcmp(metal_path.C_Str(), rough_path.C_Str()) == 0;

                bool has_spec_tex = load_specular_textures
                    && AI_SUCCESS == native_material->GetTexture(aiTextureType_SPECULAR, 0, &spec_path);
                aiColor3D spec_color;
                bool has_spec_color = AI_SUCCESS == native_material->Get(AI_MATKEY_COLOR_SPECULAR, spec_color);

                // Whether the Specular texture's R/G/B are all still meant as a
                // literal tint (the default, and the only shape SpecToMetNode's
                // conversion can meaningfully work from -- see the per-channel
                // routing further down for the alternative). Different scenes
                // pack completely different data into this legacy FBX slot (see
                // SpecularChannelUse's own comment), so this is a per-import UI
                // choice, not something auto-detected.
                bool spec_tex_all_color = specular_r == SpecularChannelUse::SpecularColor
                                        && specular_g == SpecularChannelUse::SpecularColor
                                        && specular_b == SpecularChannelUse::SpecularColor;
                bool use_specular_texture_as_color = has_spec_tex && spec_tex_all_color;
                bool use_scalar_specular_color = !has_spec_tex && has_spec_color;

                // Only take the spec/gloss (SpecToMetNode) path when the (more
                // precise, glTF-native) metallic-roughness workflow above didn't
                // already provide coverage.
                bool use_specgloss = !has_metal_tex && !has_rough_tex && (use_specular_texture_as_color || use_scalar_specular_color);

                FlowGraph::output::ptr albedo_source;

                if (AI_SUCCESS == native_material->GetTexture(aiTextureType_DIFFUSE, 0, &path)
                 || AI_SUCCESS == native_material->GetTexture(aiTextureType_BASE_COLOR, 0, &path))
                {
                   auto native_path = resolve_texture_path(directory, path.C_Str());
                    auto diff = get_texture(native_path);
                    albedo_tex = diff;
                    tex_node = make_sampling_node(graph.get(), diff, true);
                    albedo_source = tex_node->get_output(0);
                }

                else
                {
                    aiColor3D albedo;
                    native_material->Get(AI_MATKEY_COLOR_DIFFUSE, albedo);
                    auto value_node = std::make_shared<VectorNode>(vec4(albedo.r, albedo.g, albedo.b, 1));
                    graph->register_node(value_node);
                    albedo_source = value_node->get_output(0);
                }

                // Spec/gloss's own SpecToMetNode conversion below supplies
                // base_color instead, from this same albedo_source.
                if (!use_specgloss)
                    link_or_throw(albedo_source, graph->get_base_color(), "base color");

                // BC5/R8G8-encoded normal maps (the common FBX/DDS convention) only
                // store X/Y -- reconstruct Z via ReconstructNormalZNode instead of
                // linking the raw (meaningless-blue-channel) sample straight through.
                auto link_normal_texture = [&graph](TextureAsset::ptr diff)
                {
                    auto tex_node = make_sampling_node(graph.get(), diff);

                    if (is_two_channel_normal_format(diff->get_texture()->get_desc().as_texture().Format))
                    {
                        auto recon = std::make_shared<ReconstructNormalZNode>();
                        graph->register_node(recon);
                        link_or_throw(tex_node->get_output(1), recon->get_input(0), "normal texture r -> ReconstructNormalZNode");
                        link_or_throw(tex_node->get_output(2), recon->get_input(1), "normal texture g -> ReconstructNormalZNode");
                        link_or_throw(recon->get_output(0), graph->get_normals(), "reconstructed normal");
                    }
                    else
                    {
                        link_or_throw(tex_node->get_output(0), graph->get_normals(), "normal texture");
                    }
                };

                if (AI_SUCCESS == native_material->GetTexture(aiTextureType_NORMALS, 0, &path))
                {
					auto native_path = resolve_texture_path(directory, path.C_Str());
					link_normal_texture(get_texture(native_path));
                }

                else if (AI_SUCCESS == native_material->GetTexture(aiTextureType_HEIGHT, 0, &path))
                {
					auto native_path = resolve_texture_path(directory, path.C_Str());
					link_normal_texture(get_texture(native_path));
                }

                // If the specular texture isn't a uniform tint (spec_tex_all_color
                // false), individual channels can still carry real
                // metalness/roughness data -- Bistro's own README documents its
                // "Specular" texture as R=Occlusion (dropped, no AO channel exists
                // in this engine yet), G=Roughness, B=Metalness, filed under the
                // legacy FBX Specular slot instead of METALNESS/DIFFUSE_ROUGHNESS.
                // Sample once and route whichever channels the user explicitly
                // assigned those meanings to (see LoadingWindow's Specular
                // section) -- same priority as the glTF metallic-roughness
                // texture below: a real dedicated texture always wins over this.
                bool spec_provides_roughness = false;
                bool spec_provides_metallic = false;

                if (has_spec_tex && !spec_tex_all_color)
                {
                    auto spec_channel_node = make_sampling_node(graph.get(), get_texture(resolve_texture_path(directory, spec_path.C_Str())));
                    SpecularChannelUse channel_uses[3] = { specular_r, specular_g, specular_b };

                    for (int c = 0; c < 3; c++)
                    {
                        if (channel_uses[c] == SpecularChannelUse::Roughness && !has_rough_tex)
                        {
                            link_or_throw(spec_channel_node->get_output(1 + c), graph->get_roughness(), "specular channel -> roughness");
                            spec_provides_roughness = true;
                        }
                        else if (channel_uses[c] == SpecularChannelUse::Metalness && !has_metal_tex)
                        {
                            link_or_throw(spec_channel_node->get_output(1 + c), graph->get_mettalic(), "specular channel -> metallic");
                            spec_provides_metallic = true;
                        }
                    }
                }

                // glTF packs metallic-roughness into one texture: G = roughness, B = metallic.
                // Standalone (grayscale) maps are sampled from R instead.
                // (has_metal_tex/has_rough_tex/packed computed earlier, alongside
                // the spec/gloss workflow decision.)
                SamplingNode::ptr metal_rough_node;

                if (has_rough_tex)
                {
                    metal_rough_node = make_sampling_node(graph.get(), get_texture(resolve_texture_path(directory, rough_path.C_Str())));
                    metal_rough_node->get_output(packed ? 2 : 1)->link(graph->get_roughness());
                }

                else if (!use_specgloss && !spec_provides_roughness)
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
                        node = make_sampling_node(graph.get(), get_texture(resolve_texture_path(directory, metal_path.C_Str())));

                    node->get_output(packed ? 3 : 1)->link(graph->get_mettalic());
                }

                else if (!use_specgloss && !spec_provides_metallic)
                {
                    float reflectivity;

                    // AI_MATKEY_REFLECTIVITY ($mat.reflectivity, FBX's own
                    // ReflectionFactor) is already a [0,1] fraction -- unlike
                    // AI_MATKEY_SHININESS's Phong exponent above, it needs no
                    // /255 normalization (dividing it crushed any material with
                    // ReflectionFactor above ~0.4 down to near-zero metallic).
                    if (AI_SUCCESS == native_material->Get(AI_MATKEY_REFLECTIVITY, reflectivity))
                    {
                        if (reflectivity > 1)
                            reflectivity = 1;

                        auto value_node = std::make_shared<ScalarNode>(reflectivity);
                        graph->register_node(value_node);
                        value_node->get_output(0)->link(graph->get_mettalic());
                    }
                }

                // PBR Specular/Glossiness -> metallic-roughness, via the same
                // spec_to_metallic() conversion the material editor's own
                // SpecToMetNode uses (universal_material.hlsl). Supplies both
                // base_color and metallic together, since the conversion needs
                // diffuse+specular jointly to separate them.
                if (use_specgloss)
                {
                    FlowGraph::output::ptr specular_source;

                    // A 1x1 "texture" is never real per-pixel data -- some DCC
                    // exporters bake a "read channel X of some other texture"
                    // node into a flat solid-color swatch when they can't
                    // preserve that channel-selection through FBX export,
                    // which reads back here as an arbitrary, meaningless tint
                    // (confirmed on real content: a solid green 1x1 "specular"
                    // texture that was actually some other property's channel
                    // mask). Sampling a 1x1 texture is numerically identical
                    // to using a flat color everywhere, so there's no reason
                    // to prefer it over AI_MATKEY_COLOR_SPECULAR when both
                    // exist -- only fall through to using it if that's all
                    // there is.
                    bool spec_tex_is_degenerate = false;
                    bool spec_tex_is_scalar = false;
                    TextureAsset::ptr spec_tex;

                    if (has_spec_tex)
                    {
                        spec_tex = get_texture(resolve_texture_path(directory, spec_path.C_Str()));
                        auto desc = spec_tex->get_texture()->get_desc().as_texture();
                        spec_tex_is_degenerate = desc.Dimensions.x <= 1 && desc.Dimensions.y <= 1;
                        spec_tex_is_scalar = is_scalar_mask_format(desc.Format);

                        if (spec_tex_is_degenerate || spec_tex_is_scalar)
                            Log::get() << Log::LEVEL_WARNING << "AssimpLoader: specular texture \"" << spec_path.C_Str()
                                << "\" is " << desc.Dimensions.x << "x" << desc.Dimensions.y << ", format " << desc.Format.to_string()
                                << (spec_tex_is_degenerate ? " (degenerate)" : "")
                                << (spec_tex_is_scalar ? " (1-2 channel -- treating as scalar, not color)" : "")
                                << (spec_tex_is_degenerate && has_spec_color ? " -- using AI_MATKEY_COLOR_SPECULAR instead" : "")
                                << Log::endl;
                    }

                    if (has_spec_tex && !(spec_tex_is_degenerate && has_spec_color))
                    {
                        auto spec_node = make_sampling_node(graph.get(), spec_tex, true);

                        // A 1-2 channel format can't hold a real tint -- use the
                        // scalar R channel instead of the raw .rgb sample (HLSL
                        // auto-splats a scalar argument to spec_to_metallic's
                        // float3 specular parameter, so no explicit broadcast
                        // node is needed).
                        specular_source = spec_tex_is_scalar ? spec_node->get_output(1) : spec_node->get_output(0);
                    }
                    else
                    {
                        auto value_node = std::make_shared<VectorNode>(vec4(spec_color.r, spec_color.g, spec_color.b, 1));
                        graph->register_node(value_node);
                        specular_source = value_node->get_output(0);
                    }

                    auto conv = std::make_shared<SpecToMetNode>();
                    graph->register_node(conv);
                    link_or_throw(albedo_source, conv->get_input(0), "spec/gloss albedo -> SpecToMetNode");
                    link_or_throw(specular_source, conv->get_input(1), "spec/gloss specular -> SpecToMetNode");
                    link_or_throw(conv->get_output(0), graph->get_base_color(), "spec/gloss-derived base color");
                    link_or_throw(conv->get_output(1), graph->get_mettalic(), "spec/gloss-derived metallic");

                    // AI_MATKEY_GLOSSINESS_FACTOR: 0 = completely rough, 1 =
                    // perfectly smooth -- inverse of the engine's roughness.
                    float glossiness;
                    if (AI_SUCCESS == native_material->Get(AI_MATKEY_GLOSSINESS_FACTOR, glossiness))
                    {
                        auto value_node = std::make_shared<ScalarNode>(1.0f - glossiness);
                        graph->register_node(value_node);
                        link_or_throw(value_node->get_output(0), graph->get_roughness(), "spec/gloss-derived roughness");
                    }
                }

                if (AI_SUCCESS == native_material->GetTexture(aiTextureType_EMISSION_COLOR, 0, &path)
                 || AI_SUCCESS == native_material->GetTexture(aiTextureType_EMISSIVE, 0, &path))
                {
                    auto node = make_sampling_node(graph.get(), get_texture(resolve_texture_path(directory, path.C_Str())), true);

                    // KHR_materials_emissive_strength (glTF) / Maya Stingray-PBR
                    // emissive_intensity (FBX) -- an HDR multiplier on top of the
                    // [0,1] emissive texture, not present for most FBX materials.
                    float intensity;
                    if (AI_SUCCESS == native_material->Get(AI_MATKEY_EMISSIVE_INTENSITY, intensity))
                    {
                        auto intensity_node = std::make_shared<ScalarNode>(intensity);
                        graph->register_node(intensity_node);

                        auto mul_node = std::make_shared<MulNode>();
                        graph->register_node(mul_node);

                        link_or_throw(node->get_output(0), mul_node->get_input(0), "emissive texture -> intensity MulNode");
                        link_or_throw(intensity_node->get_output(0), mul_node->get_input(1), "emissive intensity -> MulNode");
                        link_or_throw(mul_node->get_output(0), graph->get_glow(), "intensity-scaled glow");
                    }
                    else
                    {
                        link_or_throw(node->get_output(0), graph->get_glow(), "emissive texture -> glow");
                    }
                }

                // Translucency (glass/water) vs. cutout: a per-pixel opacity
                // map means cutout; a flat opacity factor < 1 or a glTF
                // transmission factor means a see-through surface. The mode
                // follows from the wiring (universal_material::
                // resolve_transparency_mode): linked opacity -> Masked, linked
                // refraction (IOR) -> Translucent.
                bool translucent = false;
                {
                    auto link_scalar = [&](float value, auto target, const char* what)
                    {
                        auto node = std::make_shared<ScalarNode>(value);
                        graph->register_node(node);
                        link_or_throw(node->get_output(0), target, what);
                    };

                    float transmission = 1.0f, transmission_factor = 0, opacity_factor = 1;
                    bool has_opacity_texture = AI_SUCCESS == native_material->GetTexture(aiTextureType_OPACITY, 0, &path);

                    if (AI_SUCCESS == native_material->Get(AI_MATKEY_TRANSMISSION_FACTOR, transmission_factor) && transmission_factor > 0.0f)
                    {
                        translucent = true;
                        transmission = transmission_factor;
                    }
                    else if (!has_opacity_texture && AI_SUCCESS == native_material->Get(AI_MATKEY_OPACITY, opacity_factor) && opacity_factor < 1.0f)
                    {
                        translucent = true;
                        transmission = 1.0f - opacity_factor;
                    }

                    if (translucent)
                    {
                        float ior = 1.5f;
                        float file_ior;
                        if (AI_SUCCESS == native_material->Get(AI_MATKEY_REFRACTI, file_ior) && file_ior > 1.0f)
                            ior = file_ior;
                        link_scalar(ior, graph->get_refraction(), "IOR -> refraction");
                        link_scalar(transmission, graph->get_transmission(), "transmission");

                        // glTF KHR_materials_volume: 0 = thin-walled, same convention as ours.
                        float thickness;
                        if (AI_SUCCESS == native_material->Get(AI_MATKEY_VOLUME_THICKNESS_FACTOR, thickness) && thickness > 0.0f)
                            link_scalar(thickness, graph->get_thickness(), "thickness");

                        float attenuation_distance;
                        if (AI_SUCCESS == native_material->Get(AI_MATKEY_VOLUME_ATTENUATION_DISTANCE, attenuation_distance) && attenuation_distance > 0.0f)
                            link_scalar(attenuation_distance, graph->get_absorption_distance(), "absorption distance");
                    }
                }

                // Standalone grayscale map (R channel), same convention as the
                // roughness/metalness standalone case above.
                if (AI_SUCCESS == native_material->GetTexture(aiTextureType_OPACITY, 0, &path))
                {
                    auto node = make_sampling_node(graph.get(), get_texture(resolve_texture_path(directory, path.C_Str())));
                    node->get_output(1)->link(graph->get_opacity());
                }
                else if (!translucent)
                {
                    // Neither a dedicated opacity texture nor a scalar factor --
                    // fall back to the base color texture's own alpha channel,
                    // the standard glTF/Bistro convention ("BaseColor: RGB =
                    // color, Alpha = Opacity"). Gated on the texture's format
                    // actually being one that's chosen for real alpha content
                    // (see format_likely_has_real_alpha's own comment), so a
                    // plain opaque color texture doesn't get needlessly wired
                    // into the transparent render path.
                    if (albedo_tex && format_likely_has_real_alpha(albedo_tex->get_texture()->get_desc().as_texture().Format))
                    {
                        link_or_throw(tex_node->get_output(4), graph->get_opacity(), "base color alpha -> opacity");
                    }
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
              }
              catch (const std::exception& e)
              {
                  // link_or_throw() above is what actually throws here (a
                  // material-graph parameter type mismatch, e.g. the
                  // SpecToMetNode output-type bug this guarded against) --
                  // caught per-material rather than left to propagate: this
                  // runs on a thread_pool task and the caller only .wait()s
                  // the future, never .get()s it, so an uncaught exception
                  // here would silently vanish instead of surfacing at all.
                  // materials[i] stays null (its default from materials.
                  // resize() above) -- better one material missing than the
                  // whole import crashing or silently losing the error.
                  Log::get() << Log::LEVEL_ERROR << "AssimpLoader: material [" << i << "] in mesh ["
                      << directory.string() << "] failed to build: " << e.what() << Log::endl;
                  return false;
              }
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