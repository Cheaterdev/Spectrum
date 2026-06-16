export module Core:ResourceManager;
import :Serializer;
import :serialization;
import :Utils;
import :Log;
import :Singleton;

import stl.filesystem;
import stl.memory;
import :FileSystem;
import :FileDepender;
import :FileDataStorage;

export
{

class resource_system
{

};

template<class _resource, class _header, class _concrete>
class resource_manager_base : public Singleton<resource_manager_base<_resource, _header, _concrete>>
{
	typedef ClassLogger<resource_system> Logger;
	friend typename _concrete;

	std::map<_header, std::shared_future<std::shared_ptr<_resource>>> resources;
	std::mutex m;
	template<class ...Args>
	std::shared_ptr<_resource> get_resource(const _header& h, Args... args)
	{
		m.lock();
		auto& resource = resources[h];



		if (!resource.valid())
		{
			std::promise<std::shared_ptr<_resource>> p;
			resource = p.get_future().share();
			m.unlock();

			Logger::get() << Log::LEVEL_DEBUG << " creating new resource " << typeid(_resource).name() << " with header " << NP(typeid(_header).name(), h) << Log::endl;

			p.set_value(_concrete::create_new(h, args...));
			Logger::get() << Log::LEVEL_DEBUG << typeid(_resource).name() << " created" << Log::endl;
		}
		else
			m.unlock();


		return resource.get();
	}

	void reload_all()
	{
		m.lock();
		for (auto& r : resources)
		{
			auto resource = r.second.get();//.lock();

			if (resource)
				resource->reload_resource();
		}
		m.unlock();
	}
};


template<class T> concept CanReload =
requires (T t, T t2) {
	t = t2;
};

// Resources may opt into a cache subfolder via a static cache_subfolder().
// Used to segregate backend-specific caches (shaders/PSOs → cache/<backend>/),
// while backend-agnostic resources (textures) omit it and cache in the root.
template<class T> concept HasCacheSubfolder = requires { T::cache_subfolder(); };
template<class _resource, class _header>
class resource_manager
{
	typedef ClassLogger<resource_system> Logger;

	resource_file_depender file_depends;
	friend class resource_manager_base<_resource, _header, resource_manager<_resource, _header>>;
public:
	virtual ~resource_manager()
	{
	}

	static	std::shared_ptr<_resource> get_resource(const _header& h)
	{
		return resource_manager_base<_resource, _header, resource_manager<_resource, _header>>::get().get_resource(h);
	}

	static void reload_all()
	{
		resource_manager_base<_resource, _header, resource_manager<_resource, _header>>::get().reload_all();
	}
	bool depends_on(std::string v)
	{
		return file_depends.depends_on(v);
	}
	void reload_resource()
	{
		if (!file_depends.need_update())
			return;

		file_depends.clear();
		std::shared_ptr<_resource> result = _resource::load_native(header, file_depends);

		if (result)
		{
			load_succeded(result, header, file_depends);

			if constexpr (CanReload<_resource>)
				*((_resource*)this) = *result;

			Logger::get() << Log::LEVEL_DEBUG << "reload is good with header " << NP(typeid(_header).name(), header) << Log::endl;
		}

		else
			Logger::get() << Log::LEVEL_DEBUG << "reload is invalid" << Log::endl;
	}

	static void reset_manager()
	{
		resource_manager_base<_resource, _header, resource_manager<_resource, _header>>::reset();
	}

protected:

		SERIALIZE()
		{
			ar& NVP(header);
			ar& NVP(file_depends);
		}
	_header header;

	// Cache directory for this resource type: "cache", optionally with a
	// per-resource subfolder (e.g. backend name for shaders/PSOs).
	static std::filesystem::path cache_directory()
	{
		std::filesystem::path dir("cache");
		if constexpr (HasCacheSubfolder<_resource>)
			dir /= _resource::cache_subfolder();
		return dir;
	}

	static std::shared_ptr<_resource> create_new(const _header& header)
	{
		std::shared_ptr<_resource> result;
		std::string header_hash = Hasher::hash(header)  + ".bin";

		std::filesystem::path cache_dir = cache_directory();


	std::filesystem::path path = cache_dir / header_hash;
			std::shared_ptr<file> file = FileSystem::get().get_file(path);

		if (!file)
		{
			result = load_from_native(header);
		}
		else
		{
			FileDataStorage storage(path);//Serializer::get_stream(file->load_all());

			auto h = storage.get<_header>("header");
			auto file_depends = storage.get<resource_file_depender>("file_depends");

			if (file_depends.need_update())
			{
				result = load_from_native(h);
			}
			else
			{
				result.reset(new _resource());

				storage.get("resource",*result);
				result->file_depends = file_depends;
				result->header = h;
			}
		}

		return result;
	}

	static std::shared_ptr<_resource> load_from_native(const _header& header)
	{
		std::shared_ptr<_resource> result;
		resource_file_depender file_depends;
		result = _resource::load_native(header, file_depends);

		if (result)
			load_succeded(result, header, file_depends);
		else
			Logger::get() << Log::LEVEL_ERROR << "Can't load " << typeid(_resource).name() << " with header " << NP(typeid(_header).name(), header) << Log::endl;

		return result;
	}



	static void load_succeded(std::shared_ptr<_resource> result, const _header& header, const resource_file_depender& file_depends)
	{
		result->file_depends = file_depends;
		result->header = header;
		std::string header_hash = Hasher::hash(header)  + ".bin";
		std::filesystem::path cache_dir = cache_directory();
		std::filesystem::path path = cache_dir / header_hash;
		FileDataStorage storage(path);//Serializer::get_stream(file->load_all());

		storage.start_save();
		storage.put("header",header);
		storage.put("file_depends",file_depends);
		storage.put("resource",*result);

		storage.save();
	}


};



template<class _resource, class _header>
class resource_manager_simple
{
	typedef ClassLogger<resource_system> Logger;

	resource_file_depender file_depends;
	friend class resource_manager_base<_resource, _header, resource_manager_simple<_resource, _header>>;
public:
	static std::shared_ptr<_resource> get_resource(const _header& h)
	{
		return resource_manager_base<_resource, _header, resource_manager_simple<_resource, _header>>::get().get_resource(h);
	}

	static void reload_all()
	{
		resource_manager_base<_resource, _header, resource_manager_simple<_resource, _header>>::get().reload_all();
	}

	void reload_resource()
	{
		if (!file_depends.need_update())
			return;

		file_depends.clear();
		std::shared_ptr<_resource> result = _resource::load_native(header, file_depends);

		if (result)
		{
			load_succeded(result, header, file_depends);
			*static_cast<_resource*>(this) = *result;
			Logger::get() << Log::LEVEL_DEBUG << "reload is good with header " << NP(typeid(_header).name(), header) << Log::endl;
		}

		else
			Logger::get() << Log::LEVEL_DEBUG << "reload is invalid" << Log::endl;
	}

protected:
	_header header;

	static std::shared_ptr<_resource> create_new(const _header& header)
	{
		std::shared_ptr<_resource> result;
		std::string header_hash = Hasher::hash(header);
		result = load_from_native(header);
		return result;
	}

	static std::shared_ptr<_resource> load_from_native(const _header& header)
	{
		std::shared_ptr<_resource> result;
		resource_file_depender file_depends;
		result = _resource::load_native(header, file_depends);

		if (result)
			load_succeded(result, header, file_depends);
		else
			Logger::get() << Log::LEVEL_ERROR << "Can't load " << typeid(_resource).name() << " with header " << NP(typeid(_header).name(), header) << Log::endl;

		return result;
	}


	static void load_succeded(std::shared_ptr<_resource> result, const _header& header, const resource_file_depender& file_depends)
	{
		result->file_depends = file_depends;
		result->header = header;
	}
};


template<class _resource, class _header, class _context>
class loader
{
protected:
	typedef std::function<std::shared_ptr<_resource>(const _header&, resource_file_depender&, _context&)> loader_func;
private:
	static std::vector<loader_func> loaders;
	static bool inited;
public:

	static std::shared_ptr<_resource> get_resource(const _header& header, _context& context)
	{
		resource_file_depender depends;


		return load_native(header, depends, context);
	}
	static std::shared_ptr<_resource> load_native(const _header& header, resource_file_depender& used_files, _context& context)
	{
		if (!inited)
		{
			_resource::init_default_loaders();
			inited = true;
		}

		std::shared_ptr<_resource> result;

		for (auto& l : loaders)
		{
			result = l(header, used_files, context);

			if (result)
				return result;
		}

		return result;
	}

	static void add_loader(loader_func l)
	{
		loaders.push_back(l);
	}


};

template<class _resource, class _header, class _context>
std::vector<typename loader<_resource, _header, _context>::loader_func> loader<_resource, _header, _context>::loaders;

template<class _resource, class _header, class _context>
bool loader<_resource, _header, _context>::inited = _resource::init_default_loaders();


}