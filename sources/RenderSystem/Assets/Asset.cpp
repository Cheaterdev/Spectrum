#include "pch_render.h"
#include "Asset.h"
import Tasks;
import Debug;
import ppl;
using namespace concurrency;

AssetManager::AssetManager()
{
    has_worker = false;

    tree_folders.reset(new folder_item(L"All assets"));
    std::function<void(std::filesystem::path, folder_item::ptr)> iter;
    iter = [this, &iter](std::filesystem::path name, folder_item::ptr  w)
    {
        auto new_path = name/"";
		auto  folder_name = name.filename();
        folder_item::ptr f_item(new folder_item(folder_name));
        w->add_child(f_item);
        FileSystem::get().iterate(name, [this, f_item](file::ptr f)
        {
            AssetStorage::ptr asset = AssetStorage::try_load(f);

            if (asset)
            {
                asset->set_folder(f_item.get());
                assets[asset->header->id] = asset;
                f_item->add_asset(asset);
                EditContainer::add(asset.get());
            }
        }, false);
        FileSystem::get().iterate_dirs(new_path, std::bind(iter, std::placeholders::_1, f_item), false);
    };
    {
        auto t = CounterManager::get().start_count<folder_item>();
        iter(L"assets", tree_folders);
    }
 
}

AssetManager::~AssetManager()
{
    //  std::lock_guard<std::mutex> g(m);
    tree_folders = nullptr;
    /*  while (assets.size())
      {
          Log::get() << "delete " << assets.begin()->second->get_name() << Log::endl;
          assets.erase(assets.begin());
      }
    */
    funcs.clear();
}

 void AssetManager::add_func(std::function<void()> f)
{
	std::lock_guard<std::mutex> g(m);
	funcs.emplace_back(f);
}

 void AssetManager::add_preview(Asset::ptr asset)
{
	std::lock_guard<std::mutex> g(update_preview_mutex);
	

	auto my_task = [this, asset]() {
		try {
		auto& preview = asset->holder->get_preview();

		if (!preview || !preview->is_rt())
		{
			Render::Texture::ptr new_preview;
			new_preview.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_FLOAT, 256, 256, 1, 0, 1, 0, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)));
			asset->holder->editor->preview = new_preview;
		}

		asset->update_preview(asset->holder->get_preview());
		asset->holder->on_preview(asset->holder->get_preview());
		}
		catch (const std::system_error& e) { Log::get() << Log::LEVEL_ERROR << e.what() << Log::endl; }
	};

	if (task_inited)
		last_update_task = last_update_task.then(my_task);
	else
		last_update_task = create_task(my_task);

	task_inited = true;
	/*update_preview.push(a);

	if (!has_worker)
	{
		has_worker = true;
		auto& t = thread_pool::get().enqueue([this]()
		{
			while (true)
			{
				Asset::ptr asset;
				{
					std::lock_guard<std::mutex> g(update_preview_mutex);

					if (update_preview.empty()) { has_worker = false; break; }

					asset = update_preview.front();
					update_preview.pop();
				}
				auto& preview = asset->holder->get_preview();

				if (!preview || !preview->is_rt())
				{
					Render::Texture::ptr new_preview;
					new_preview.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_FLOAT, 256, 256, 1, 0, 1, 0, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)));
					asset->holder->editor->preview = new_preview;
				}

				asset->update_preview(asset->holder->get_preview());
				asset->holder->on_preview(asset->holder->get_preview());
			}
		});
		//  t.wait();
	}*/
}

 AssetStorage::ptr AssetManager::get_storage(Guid id)
{
	//std::lock_guard<std::mutex> g(m);
	auto it = assets.find(id);

	if (it != assets.end())
		return assets[id];

	return nullptr;
}

 void AssetManager::reload_resources()
{
	for (auto && a : assets)
		a.second->get_asset()->reload_resource();
}


 folder_item::ptr AssetManager::get_folders()
{
	return tree_folders;
}

 bool AssetManager::promt(EditObject * obj)
{
	AssetStorage* asset = static_cast<AssetStorage*>(obj);
	return msg_box_func(convert(asset->get_name()));
}

 void AssetManager::tact()
{
	std::vector<std::function<void()>> copy;
	{
		std::lock_guard<std::mutex> g(m);
		copy = funcs;
		funcs.clear();
	}

	for (auto f : copy)
		f();
}

 void AssetManager::save(EditObject * obj)
{
	// std::lock_guard<std::mutex> g(m);
	//  funcs.push_back([obj]()
	//  {
	AssetStorage* asset = static_cast<AssetStorage*>(obj);
	create_task([asset]()
	{
		asset->save();
		asset->on_save();
	});
	//  FileSystem::get().save_data("assets\\" + asset->name + ".asset", Serializer::serialize(asset->get_ptr()));
	//  });
}

 void AssetManager::add_listener(AssetListener * l)
{
	AssetManagerHolder::add_listener(l);

	for (auto a : assets)
		l->on_add_asset(a.second.get());
}

 void AssetManager::remove_listener(AssetListener * l)
{
	for (auto a : assets)
		l->on_remove_asset(a.second.get());

	AssetManagerHolder::remove_listener(l);
}


//////////////////////////////////////////////////////////////////////////
void Asset::save()
{
    if (holder)	holder->save();
}


void AssetReferenceBase::destroy()
{
    if (owner)
        owner->unregister_reference(this);
}

 const AssetHolder * AssetReferenceBase::get_owner() const
{
	return owner;
}

void AssetReferenceBase::init()
{
    if (base_asset)
        base_asset->add_reference(this);

    if (owner)
        owner->register_reference(this);
}
/*
AssetReferenceBase::AssetReferenceBase(AssetReferenceBase&& other) : owner(other._owner)
{
    base_asset = other.base_asset;

    if (base_asset)
        base_asset->add_reference(this);

    if (owner)
        owner->register_reference(this);
}*/
AssetReferenceBase::AssetReferenceBase(const AssetReferenceBase& other) : owner(other.owner)
{
    base_asset = other.base_asset;

    if (base_asset)
        base_asset->add_reference(this);

    if (owner)
        owner->register_reference(this);
}

AssetReferenceBase::AssetReferenceBase(std::shared_ptr<Asset> asset , AssetHolder* _owner): owner(_owner)
{
    base_asset = asset;

    if (owner&&base_asset)
        base_asset->add_reference(this);

    if (owner)
        owner->register_reference(this);
}

AssetReferenceBase::~AssetReferenceBase()
{
    if (owner&&base_asset)
        base_asset->erase_reference(this);

    if (owner)
        owner->unregister_reference(this);

    // auto name = base_asset->name;
//   Log::get() << 1 << " " << base_asset.get() << " " << base_asset.use_count() << " " << name << Log::endl;
    base_asset = nullptr;
//   Log::get() << 2 << " " << name << Log::endl;
}

 void Asset::add_reference(AssetReferenceBase * ref)
{
	m.lock();
	references.insert(ref);
	m.unlock();
}

 void Asset::erase_reference(AssetReferenceBase * ref)
{
	m.lock();
	references.erase(ref);
	m.unlock();
}

Asset::Asset()
{
}

void Asset::try_register()
{
    AssetManager::get().add_asset(ptr(this));
}

AssetStorage::ptr Asset::register_new(std::wstring name, Guid g)
{
    if (name.empty())
        name = this->name;

   return AssetManager::get().add_asset(ptr(this), name, g);
}

Asset::~Asset()
{
}

void Asset::update_preview(Render::Texture::ptr)
{
}
 void Asset::set_name(std::wstring name)
{
	this->name = name;
}
/*
void Asset::set_name(std::string name)
{
	holder->name = name;
	mark_changed();
}

void Asset::set_id(Guid id)
{
	if (!holder->id.is_good())
		holder->id = id;
}
*/
Guid Asset::get_id()
{
    return holder->header->id;
}

std::wstring Asset::get_name()
{
	return name;// holder->get_name();
}

void Asset::mark_changed()
{
    if (holder)
        holder->mark_changed();
}

void  Asset::start_changing_contents()
{
	changing_state = true;
}
void  Asset::end_changing_contents()
{
	changing_state = false;

	mark_contents_changed();
}


void Asset::mark_contents_changed()
{
	if (!changing_state)
	{
		if (holder)
			holder->mark_contents_changed();

		std::set<AssetReferenceBase*> copy = references;

		for (auto && r : copy)
			r->owner->on_asset_change(get_ptr());
	}
}

/*
template<class T>
template<class Archive>
void AssetReference<T>::serialize(Archive &ar, const unsigned int)
{
	ar &NVP(boost::serialization::base_object<AssetReferenceBase>(*this));

	if (Archive::is_loading::value)
	{
		Guid id;
		ar &NVP(id);

		if (id.is_good())
			asset = std::static_pointer_cast<T>(AssetManager::get().get(id));
	}
	else
	{
		if (asset)
		{
			if (asset->is_changed())
			{
				AssetManager::get().save(asset.get());
			}

			ar &NVP(asset->get_id());
		}
		else
			ar &NVP(Guid());
	}

	//        ar &NVP(data);
}*/
/*
template<class T>
AssetReference<T>::AssetReference(const std::shared_ptr<T> &_asset) : asset(const_cast<std::shared_ptr<T> &>(_asset))
{

}
*/

bool  Asset::is_changed()
{
    if (holder)
        return holder->is_changed();

    return false;
}

 AssetStorage::AssetStorage(file::ptr f)
{
	 my_file = f;

//	stream = str;
//	archive = arch;
	file_path = f->file_name;
	header.create_func = [this]()->Header
	{
		std::lock_guard<std::mutex> g(archive_mutex);
		auto archive = get_archive();
		auto entry = archive->GetEntry("header");
		auto res = Serializer::deserialize<Header>(*entry->GetDecompressionStream());
		entry->CloseDecompressionStream();
		return *res;
	};
	editor.create_func = [this]()->Editor
	{
		std::lock_guard<std::mutex> g(archive_mutex);
		auto archive = get_archive();
		auto entry = archive->GetEntry("editor");
		auto res = Serializer::deserialize<Editor>(*entry->GetDecompressionStream());
		entry->CloseDecompressionStream();
		on_preview(res->preview);
		return *res;
	};

	on_preview.default_state = [this](std::function<void(const Render::Texture::ptr&)> f)
	{
		create_task([f, this]()
		{
			f(editor->preview);
		});
	};
}

 void AssetStorage::set_folder(folder_item * f)
{
	folder = f;
}

AssetStorage::AssetStorage(Asset::ptr _asset) : asset(_asset)
{
    asset->holder = this;
    header.set(Header());
    editor.set(Editor());

    if (!header->id.isValid())
        header->id = xg::newGuid();

    if (header->name.empty())
        header->name = convert(to_string(header->id));

    header->type = asset->get_type();
    std::wstring s_type = L"";

    if (header->type == Asset_Type::MATERIAL)
        s_type = L"Materials";

    if (header->type == Asset_Type::MESH)
        s_type = L"Meshes";

    if (header->type == Asset_Type::TEXTURE)
        s_type = L"Textures";

    if (header->type == Asset_Type::BINARY)
        s_type = L"Binary";

    if (header->type == Asset_Type::TILED_TEXTURE)
        s_type = L"TiledTextures";

    file_path = std::filesystem::path(L"assets") / s_type / (header->name + L"_" + convert(to_string(header->id)) + L".asset");
    folder = AssetManager::get().get_folders()->get_folder(file_path).get();
    folder->add_asset(ptr(this));
    update_preview();
    mark_changed();
    on_preview.default_state = [this](std::function<void(const Render::Texture::ptr&)> f)
    {
        f(editor->preview);
    };
}

 AssetStorage::~AssetStorage()
{
	if (asset)
		asset->shutdown();

	asset = nullptr;
	
}

 folder_item * AssetStorage::get_folder()
{
	return folder;
}

 AssetStorage::ptr AssetStorage::try_load(file::ptr f)
{
	
	ZipArchive::Ptr archive = ZipArchive::Create(f->get_new_stream());

	if (archive->GetEntry("header"))
		if (archive->GetEntry("editor"))
			if (archive->GetEntry("asset"))
			{
				archive = nullptr;

				return AssetStorage::ptr(new AssetStorage(f));
			}
				

	return nullptr;
}

 const Render::Texture::ptr & AssetStorage::get_preview()
{
	return editor->preview;
}

void AssetStorage::update_preview()
{
    AssetManager::get().add_preview(get_asset());

    //.add_func([this]() { get_asset()->update_preview(preview);});
    //	AssetManager::get(). get_asset()->update_preview(preview);
//	thread_pool::get().enqueue();
}
 std::wstring AssetStorage::get_name()
{
	return header->name;
}
 Asset_Type AssetStorage::get_type()
{
	return header->type;
}
 bool AssetStorage::need_update_preview()
{
	return !editor->preview.get() || contents_changed;
}
 void AssetStorage::mark_contents_changed()
{
	contents_changed = true;
	update_preview();
}
void  AssetStorage::get_asset(std::function<void(Asset::ptr)> f)
{
    if (asset)
    {
        f(asset);
        return;
    }

    create_task([this, f]()
    {
        f(load_asset().get());
    });
}
std::future<Asset::ptr> AssetStorage::load_asset()
{
    if (asset)
    {
        std::promise<Asset::ptr> p;
        p.set_value(asset);
        return p.get_future();
    }

    std::lock_guard<std::mutex> g(m);

    if (asset)
    {
        std::promise<Asset::ptr> p;
        p.set_value(asset);
        return p.get_future();
    }

	auto archive = get_archive();

    if (archive)
    {
        std::vector<task<bool>> tasks;

        for (auto r : header->references)
        {
            tasks.emplace_back(create_task([r]()
            {
                try
                {
                    std::stringstream s;
                    s << "loading " << r;
                    auto task = TaskInfoManager::get().create_task(convert(s.str()));
                    auto storage = AssetManager::get().get_storage(r);

                    if (!storage)
                        return false;

                    storage->load_asset().get();
                }

                catch (std::exception e)
                {
                    Log::get() << "asset loading fail" << e.what() << Log::endl;
                }

                return true;
            }));
        }

        std::shared_ptr<std::promise<Asset::ptr>> p(new std::promise<Asset::ptr>);
        when_all(begin(tasks), end(tasks)).then([this, p,archive](std::vector<bool> results)
        {
			std::lock_guard<std::mutex> g(archive_mutex);

            try
            {
                for (auto r : results)
                    if (!r)
                    {
                        p->set_value(nullptr);
                        return;
                    }

                auto t = CounterManager::get().start_count<AssetStorage>(convert(header->name));
			

                auto entry = archive->GetEntry("asset");

                if (!entry)
                    return;

                {
                    auto dec_stream = entry->GetDecompressionStream();
				
                    if (!dec_stream)
                    {
                        Log::get() << "stream is NULL: " << convert(header->name) << Log::endl;
                        p->set_value(nullptr);
                        return;
                    }
					if (!dec_stream||dec_stream->bad())
					{
						Log::get() << "asset loading fail" << Log::endl;
					}
                    auto s_stream = Serializer::get_stream(*dec_stream);
                    s_stream >> asset;
                    asset->holder = this;
                }
                entry->CloseDecompressionStream();
            }
            catch (std::exception e)
            {
                Log::get() << "asset loading fail" << e.what() << Log::endl;
            }

            p->set_value(asset);
        });
        return p->get_future();
    }
	
	throw std::exception();
}

 bool AssetStorage::is_loaded()
{
	return !!asset;
}

Asset::ptr AssetStorage::get_asset()
{
    if (asset)
        return asset;

    return load_asset().get();
}

void AssetStorage::save()
{
    auto task = TaskInfoManager::get().create_task(std::wstring(L"saving") + file_path.generic_wstring());

    try
    {
		std::lock_guard<std::mutex> g(archive_mutex);

       auto archive = ZipArchive::Create();
        header->references = asset->get_reference_ids();
        DataPacker::create_entry(archive, "header", Serializer::serialize(*header.get()));
        DataPacker::create_entry(archive, "editor", Serializer::serialize(*editor.get()));
        DataPacker::create_entry(archive, "asset", Serializer::serialize(asset));
        FileSystem::get().save_data(file_path, DataPacker::zip_to_string(archive));
    }

    catch (const std::exception& e)
    {
        Log::get() << Log::LEVEL_ERROR << "Cant save asset" << asset->get_name() << ": " << e.what() << Log::endl;
    }
}

 void AssetManagerHolder::on_add_asset(AssetStorage * a)
{
	for (auto l : listeners)
		l->on_add_asset(a);
}

 void AssetManagerHolder::on_remove_asset(AssetStorage * a)
{
	for (auto l : listeners)
		l->on_remove_asset(a);
}

 void AssetManagerHolder::add_listener(AssetListener * l)
{
	listeners.insert(l);
}

 void AssetManagerHolder::remove_listener(AssetListener * l)
{
	listeners.erase(l);
}

 void AssetHolder::register_reference(AssetReferenceBase * r)
{
	m.lock();
	assets.insert(r);
	m.unlock();
}

 void AssetHolder::unregister_reference(AssetReferenceBase * r)
{
	m.lock();
	assets.erase(r);
	m.unlock();
}

 void AssetHolder::on_asset_change(std::shared_ptr<Asset> asset)
{
}

 std::set<Guid> AssetHolder::get_reference_ids()
{
	 std::set<Guid> r;
	m.lock();

	for (auto a : assets)
		if (a)
			r.insert(a->get_id());

	m.unlock();
	return r;
}

std::wstring  folder_item::get_name() const
{
	return name.generic_wstring();
}

void folder_item::add_asset(AssetStorage::ptr a)
{
	m.lock();
	assets.push_back(a);
	m.unlock();
}

void folder_item::iterate_assets(std::function<void(AssetStorage::ptr)> f)
{
	m.lock();

	for (auto && a : assets)
		f(a);

	m.unlock();
}

folder_item::ptr folder_item::get_folder(std::filesystem::path name)
{
	std::lock_guard<std::mutex> g(m);
//	name = name.substr(0, name.find_last_of(L"\\") + 1);
//	int delim = name.find_first_of(L"\\");

	if(std::filesystem::is_regular_file(name))
		return get_ptr();

	auto folder_name = *name.begin();// name.substr(0, delim);
	if(folder_name==name)
		return get_ptr();
	auto other_name = std::filesystem::relative(name, folder_name);

	//if (folder_name.empty())
	//	return get_ptr();

	for (auto p : childs)
	{
		if (p->get_name() == folder_name)
			return p->get_folder(other_name);
	}

//	std::wstring next_folder_name = other_name.substr(0, other_name.find_first_of(L"\\"));
	folder_item::ptr p(new folder_item(folder_name));
	add_child(p);
	return p->get_folder(other_name);
	//	tree_folders->
}
