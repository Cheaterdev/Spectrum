#pragma once
import EditObject;
import Utils;
import Serializer;
import Texture;
import serialization;
import Tree;
import ZipLib;
import Data;
import stl.memory;

enum class Asset_Type : int
{
	UNKNOWN,
	TEXTURE,
	MESH,
	MATERIAL,
	BINARY,
	TILED_TEXTURE
};


class Asset;
class AssetManagerHolder;
class AssetHolder;
class AssetManager;
class AssetStorage;

template<class T>
struct AssetHoldable
{
	T* asset = nullptr;

};

class AssetListener
{
		friend class AssetManagerHolder;
		friend class AssetManager;

	protected:
		virtual void on_add_asset(AssetStorage*) = 0;
		virtual void on_remove_asset(AssetStorage*) = 0;
};

class AssetManagerHolder
{
		std::set<AssetListener*> listeners;

	protected:
		void on_add_asset(AssetStorage* a);
		void on_remove_asset(AssetStorage* a);
	public:

		virtual void add_listener(AssetListener* l);

		void remove_listener(AssetListener* l);
};

class AssetHolder;
class AssetReferenceBase
{
		friend class AssetHolder;
		friend class Asset;
	protected:
		std::shared_ptr<Asset> base_asset;
		AssetHolder*  owner;
		void init();
	

	public:
		void destroy();
		const  AssetHolder* get_owner() const;
		virtual Guid get_id() = 0;

		const bool operator!=(const AssetReferenceBase& r)const 
		{
			return base_asset != r.base_asset;
		}

		const bool operator==(const AssetReferenceBase& r) const
		{
			return base_asset == r.base_asset;
		}
		AssetReferenceBase(const AssetReferenceBase& other);

		AssetReferenceBase(AssetHolder* owner = nullptr);
		virtual ~AssetReferenceBase();
};


template<class T>
class AssetReference : public AssetReferenceBase
{

		friend class AssetHolder;
		std::shared_ptr<T> asset;

		LEAK_TEST(AssetReference)

	public:
		AssetReference(AssetHolder* owner) : AssetReferenceBase( owner)
		{
		}

		AssetReference(const AssetReference<T>& other);
		virtual ~AssetReference();
		T* operator->()
		{
			return asset.get();
		}
		std::shared_ptr<T> operator*()
		{
			return asset;
		}
		/* bool is_same_internal(const void* _asset) const override
		 {
			 return &asset == _asset;
		 }
		*/

		void operator=(const AssetReference<T> &r)
		{
			destroy();
			base_asset = r.base_asset;
			asset = r.asset;
			init();
		}

		template<class T>
		void operator=(std::shared_ptr<T> asset)
		{
			destroy();
			base_asset = asset;
			this->asset = asset;
			init();
		}


		/*void operator=(AssetReference<T>&& r)
		{
			base_asset = r.base_asset;
			asset = r.asset;
			init();
			
		}*/
		const bool operator!=(const AssetReference<T>& r)const
		{
			return base_asset != r.base_asset;
		}

		const bool operator==(const AssetReference<T>& r) const
		{
			return base_asset == r.base_asset;
		}


		/*     void operator=(AssetReference<T>&& r)
			 {
				 base_asset = r.base_asset;
				 asset = r.asset;
				 init();
			 }*/
		virtual Guid get_id()
		{
			if (asset)
				return asset->get_id();

			return Guid();
		}
		AssetReference() = default;

	private:
	
		SERIALIZE();

		// We could define load_and_construct internally:
 template <class Archive>
 static void load_and_construct( Archive & ar, cereal::construct<AssetReference<T>> & construct )
 {
	std::shared_ptr<AssetHolder> owner;
	 ar& NVP(owner);
	construct(owner.get());
 }
 
};

template <class T>
class AssetReferenceVector : public std::vector<T>
{
	AssetHolder* holder;
public:
	AssetReferenceVector(AssetHolder* holder) :holder(holder) {};


};
class AssetHolder : public SharedObject<AssetHolder>
{

		friend class Asset;

		friend class AssetReferenceBase;
		std::mutex m;
		std::set<AssetReferenceBase*> assets;
		LEAK_TEST(AssetHolder)
			void register_reference(AssetReferenceBase* r);

		void unregister_reference(AssetReferenceBase* r);
	public:

		template<class T>
		AssetReference<T> register_asset(s_ptr<T> link)
		{
			auto r = AssetReference<T>(this);
			r = link;
			return r;
		}
		
//		virtual std::shared_ptr<Asset> get_asset_holder() = 0;
	protected:
		virtual void on_asset_change(std::shared_ptr<Asset> asset);
	public:
		std::set<Guid> get_reference_ids();

		SERIALIZE()
		{
		}
		virtual ~AssetHolder()
		{
		}

};
class AssetStorage;
class Asset : public SharedObject<Asset>, public AssetHolder, public EditObject, public virtual Events::prop_handler
{
		friend class AssetManager;
		friend class AssetStorage;
		friend class AssetHolder;
		friend class AssetReferenceBase;
	protected:
		LEAK_TEST(Asset)

		std::wstring name;
		std::mutex m;
		std::set<AssetReferenceBase*> references;
		virtual void shutdown() {}
		AssetStorage* holder = nullptr;
		std::set<std::wstring> additional_files;

		void add_reference(AssetReferenceBase* ref);

		void erase_reference(AssetReferenceBase* ref);

		
		bool changing_state = false;

		void start_changing_contents();
		void end_changing_contents();


public:
	using SharedObject<Asset>::get_ptr;
		Asset();

		virtual void try_register();
		std::shared_ptr<AssetStorage>  register_new(std::wstring name = L"", Guid g = Guid());
		std::wstring get_name();

		void set_name(std::wstring name);
		Guid get_id();

		//void set_id(Guid id);

		//  void set_name(std::string name);

		virtual void update_preview(Graphics::Texture::ptr);
	

		using ptr = s_ptr<Asset>;
		using ref = AssetReference<Asset>;

		virtual Asset_Type get_type()
		 {
			 return Asset_Type::UNKNOWN;
		 }


		virtual ~Asset();;

		virtual void mark_changed() override;
		virtual void mark_contents_changed();

		virtual bool is_changed() override;
		virtual void save() override;

		virtual void reload_resource() {};
		virtual void load(std::istream&& s)
		{
		}

		virtual void save(std::ostream&& s)
		{
		}

	private:
		SERIALIZE()
		{
			 SAVE_PARENT(AssetHolder);
			ar& NVP(name);
			ar& NVP(additional_files);
		}
};
//BOOST_SERIALIZATION_ASSUME_ABSTRACT(Asset);


class folder_item;


class AssetStorage : public SharedObject<AssetStorage>, public EditObject
{
		LEAK_TEST(AssetStorage)


		friend class Asset;
		friend class AssetManager;


		struct Header
		{
				Guid id;
				std::wstring name;

				std::set<Guid> references;
				std::set<std::wstring> additional_files;
				Asset_Type type;

			private:
				SERIALIZE()
				{
					ar& NVP(id);
					ar& NVP(name);
					ar& NVP(references);
					ar& NVP(additional_files);
					ar& NVP(type);
				}


		};

		struct Editor
		{
				Graphics::Texture::ptr preview;
			private:
				SERIALIZE()
				{
				   ar & NVP(preview);
				}
		};



		Void<Header> header;
		Void<Editor> editor;
		Asset::ptr asset;


		std::weak_ptr<ZipArchive> archive;

		std::mutex stream_mutex;
		ZipArchive::Ptr get_archive()
		{
			auto res = archive.lock();
			if (!res)
			{
				std::lock_guard<std::mutex> g(stream_mutex);
				res = archive.lock();
				if (res) return res;
		
				res = ZipArchive::Create(my_file->get_new_stream());

				return res;

			}

			return res;
		}
		folder_item* folder;

		std::filesystem::path file_path;
		std::mutex m;
		std::mutex archive_mutex;

	  //  std::shared_ptr<std::istream> stream;
		file::ptr my_file;
		AssetStorage(file::ptr f);
		void set_folder(folder_item* f);

		bool contents_changed = false;
	public:
		Events::Event<const Graphics::Texture::ptr&> on_preview;
		using ptr = s_ptr<AssetStorage>;
		AssetStorage(Asset::ptr _asset);


		virtual ~AssetStorage();
		folder_item* get_folder();
		static AssetStorage::ptr try_load(file::ptr f);

		 std::set<Guid> get_references() 
	{
		return header->references;
	}
		const Graphics::Texture::ptr& get_preview();

		virtual void update_preview();

		std::wstring get_name();
		Asset_Type get_type();

		bool need_update_preview();

		void mark_contents_changed();

		bool is_loaded();

		Asset::ptr get_asset();
		std::future<Asset::ptr> load_asset();

		void get_asset(std::function<void(Asset::ptr)>);
		void save() override;

};

class folder_item : public ::tree<folder_item, my_unique_vector<std::shared_ptr<folder_item>>>
{
	std::filesystem::path name;
		std::vector<AssetStorage::ptr> assets;
		std::mutex m;

		LEAK_TEST(folder_item)

	public:

		folder_item(std::filesystem::path name)
		{
			this->name = name;
		}
		using ptr = s_ptr<folder_item>;
		virtual std::wstring get_name() const override;

		void add_asset(AssetStorage::ptr a);

		void iterate_assets(std::function<void(AssetStorage::ptr)> f);
		folder_item::ptr get_folder(std::filesystem::path name);
};



class AssetManager : public Singleton<AssetManager>, public EditContainer, public AssetManagerHolder
{
		friend class Singleton<AssetManager>;
		friend class AssetStorage;

		AssetManager();
		~AssetManager();
		std::map<Guid, AssetStorage::ptr> assets;
		std::mutex m;
		std::vector<std::function<void()>> funcs;
		folder_item::ptr tree_folders;

		LEAK_TEST(AssetManager)

			void add_func(std::function<void()> f);

		std::mutex update_preview_mutex;
		std::queue<Asset::ptr> update_preview;
		std::atomic_bool has_worker;

		void add_preview(Asset::ptr a);
		concurrency::task<void> last_update_task;
		bool task_inited = false;
	public:

		using Singleton<AssetManager>::get;
		template <class T = Asset>
		s_ptr<T> get(Guid id)
		{
			//std::lock_guard<std::mutex> g(m);
			auto it = assets.find(id);

			if (it != assets.end())
				return assets[id]->get_asset()->get_ptr<T>();

			return nullptr;
		}
		template<class T>
		typename T::ptr find(std::function<bool(typename T::ptr&)> f)
		{
			for (auto & storage : assets)
			{
				if (storage.second->get_type() == T::TYPE)
				{
					auto asset = storage.second->get_asset()->get_ptr<T>();
					if (asset && f(asset))return asset;
				}
		
			}

			return nullptr;
		}

		template<class T>
		typename T::ptr find_by_name(std::wstring name)
		{
			for (auto& storage : assets)
			{
				if (storage.second->get_type() == T::TYPE)
				{
					auto asset = storage.second->get_asset()->get_ptr<T>();
					if (asset&&asset->get_name()== name)return asset;
				}

			}

			return nullptr;
		}


		typename AssetStorage::ptr find_storage_by_name(std::wstring name)
		{
			for (auto& storage : assets)
			{
			
			if (storage.second->get_name() == name) return storage.second;

			}

			return nullptr;
		}


		template <class T = Asset>
		s_ptr<T> get(Guid id, std::function<T*()> create_func)
		{
			//std::lock_guard<std::mutex> g(m);
			auto it = assets.find(id);

			if (it != assets.end())
				return assets[id]->get_asset()->get_ptr<T>();

			auto asset = create_func();
			asset->register_new(asset->name, id);
			asset->save();
			return asset->get_ptr<T>();
		}

		AssetStorage::ptr get_storage(Guid id);
		void reload_resources();


		std::function<bool(std::string)> msg_box_func = [](std::string str)
		{
			return MessageBoxA(0, str.c_str(), "message", MB_YESNO) == IDOK;
		};

		std::map<Guid, AssetStorage::ptr> get_assets()
		{
			return assets;
		}

		template<class T>
		AssetStorage::ptr add_asset(s_ptr<T> asset, std::wstring name = L"", Guid id = Guid())
		{
			auto storage = (new AssetStorage(asset))->get_ptr();

			if (!id.isValid())
				id = xg::newGuid();

			if (name.empty())
				name = convert(to_string(id));

			storage->header->id = id;
			storage->header->name = name;
			assets[asset->get_id()] = storage;
			EditContainer::add(storage.get());
			on_add_asset(storage.get());
			return storage;

		}
		folder_item::ptr get_folders();


		virtual bool promt(EditObject* obj);

		void tact();
		virtual void save(EditObject* obj);

		virtual void add_listener(AssetListener* l);

		virtual void remove_listener(AssetListener* l);
};

//////////////////////////////////////////////////////////////////////////

/*  AssetReference(AssetReference<T>&& other) : AssetReferenceBase(std::forward<AssetReferenceBase>(other))
{
asset = other.asset;
}*/
template<class T>
inline AssetReference<T>::AssetReference(const AssetReference<T>& other) : AssetReferenceBase(other)
{
	asset = other.asset;
}

template<class T>
inline AssetReference<T>::~AssetReference()
{
	//destroy();

}


/*
test case only

std::vector<unsigned char> ids(16);
ids[0] = 1;
Guid material_tester_id(ids);
MeshAsset::ptr asset_ptr = AssetManager::get().get<MeshAsset>(material_tester_id);

if (!asset_ptr)
{
MeshAsset* asset = (new MeshAsset(L"resources\\mitsuba\\mitsuba-sphere.obj"));
asset->register_new(L"material_tester", material_tester_id);
//  asset->save();
asset_ptr = asset->get_ptr<MeshAsset>();
}

material_tester.reset(new MeshAssetInstance(asset_ptr));


*/



template<class T>
class EngineAsset
{
	//static std::map<AssetStorage::ptr, std::function<Asset*()>> assets;
	typename T::ptr asset;
	std::function<T*()> creator;
	Guid id; std::wstring name;
	std::mutex m;
public:
	EngineAsset(std::wstring name, std::function<T*()> creator) :name(name),creator(creator)
	{
		std::lock_guard<std::mutex> g(m);
		auto data = Hasher::hash(convert(name));

		std::array<unsigned char, 16> ids;
		for (size_t i = 0; i < data.size(); i++)
			ids[i] = data[i];

		for (size_t i = data.size(); i < 16; i++)
			ids[i] = 0;
		id = Guid(ids);

	}


	typename T::ptr get_asset()
	{
		std::lock_guard<std::mutex> g(m);

		if(!asset)
			asset = AssetManager::get().get<T>(id);

		if (!asset&&creator)
		{
			auto asset_ptr = creator();
			asset_ptr->register_new(name, id);
			asset = asset_ptr->get_ptr<T>();
		}

		return asset;
	}
};



template<class T>
template<class Archive>
void AssetReference<T>::serialize(Archive& ar, const unsigned int)
{
	// save data required to construct instance
	auto owner_ptr = owner?owner->get_ptr():nullptr;
	ar& NVP(owner_ptr);

	//SAVE_PARENT(AssetReferenceBase);
	if (Archive::is_loading::value)
	{
		owner = owner_ptr.get();
		Guid guid;
		ar& NVP(guid);

		if (guid.isValid())
		{
			auto storage = AssetManager::get().get_storage(guid);

			if (!storage)
				throw std::exception("cant load reference, no asset storage");
			if (!storage->is_loaded())
				Log::get() << "UNWANTED BEHAVIOUR FOR " << "->" << convert(storage->get_name()) << Log::endl;

			auto ass = storage->get_asset();

			if (!ass)
				throw std::exception("cant load reference");

			asset = ass->get_ptr<T>();
			base_asset = asset;
			init();
		}
	}

	else
	{

		Guid guid;

		if (asset) guid = asset->get_id();

		ar& NVP(guid);
	}

	//        ar &NVP(data);
}