#pragma once
#include "utils/utils_macros.h"

#include "Serialization/Serializer.h"
import Utils;
import Log;
import Singleton;

import stl.filesystem;
import stl.memory;
import FileSystem;

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
		//	bool first = resources.find(h) == resources.end();
		auto& resource = resources[h];
		// std::shared_ptr<_resource> resource = resource_weak.lock();



		if (!resource.valid())
		{
			std::promise<std::shared_ptr<_resource>> p;
			resource = p.get_future().share();
			m.unlock();

			Logger::get() << Log::LEVEL_DEBUG << " creating new resource " << typeid(_resource).name() << " with header " << NP(typeid(_header).name(), h) << Log::endl;

			p.set_value(_concrete::create_new(h, args...));
			Logger::get() << Log::LEVEL_DEBUG << typeid(_resource).name() << " created" << Log::endl;
			//   resource_weak = resource;     
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


class resource_file_depender
{
	struct depender
	{
		std::wstring file_name;
		std::filesystem::file_time_type modify_time;

		bool need_update();
	private:

		inline const std::wstring& get_name() const { return file_name; }
SERIALIZE()
		{
			ar& NVP(file_name)& NVP(modify_time);
		}
	};
	std::vector<depender> files;
public:

	inline const std::vector<depender>& get_files() const { return files; }

	void add_depend(std::shared_ptr<file> _file);
	bool need_update();
	void clear();
	bool depends_on(std::string v);
private:
	SERIALIZE()
	{
		ar& NVP(files);
	}
};

template<class T> concept CanReload =
requires (T t, T t2) {
	t = t2;
};
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

			//	swap(*static_cast<_resource*>(this), *result);
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
	_header header;

	static std::shared_ptr<_resource> create_new(const _header& header)
	{
		std::shared_ptr<_resource> result;
		std::wstring header_hash = convert(Hasher::hash(header));
		std::shared_ptr<file> file = FileSystem::get().get_file(L"cache\\" + header_hash + L".bin");

		if (!file)
		{
			result = load_from_native(header);
		}
		else
		{
			auto stream = Serializer::get_stream(file->load_all());
			_header h;
			stream >> h;
			resource_file_depender file_depends;
			stream >> file_depends;

			if (file_depends.need_update())
			{
				result = load_from_native(h);
			}
			else
			{
				result.reset(new _resource());
				stream >> *result;
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
		std::string header_hash = Hasher::hash(header);
		Serializer::serialization_ostream stream;

		try
		{
			stream << header << file_depends << *result;
			FileSystem::get().save_data(L"cache\\" + convert(header_hash) + L".bin", Serializer::serialize(stream, true));
		}

		catch (std::exception& e)
		{
			Log::get().crash_error(0, e.what());
		}
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


/*

template<class _resource, class _header>
class loader_provider
{
	static std::shared_ptr<_resource> operator()(const _header& header, resource_file_depender &used_files)
	{
		return std::shared_ptr<_resource>();
	}
};*/

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

