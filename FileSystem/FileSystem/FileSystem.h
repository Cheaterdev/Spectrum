struct file;

class file_provider
{

    public:
        using ptr = s_ptr<file_provider>;

        virtual std::string load_all(file*) = 0;
        virtual std::shared_ptr<file> get_file(std::wstring) = 0;
        virtual std::shared_ptr<istream> create_stream(std::wstring) = 0;

        virtual bool save_data(std::wstring, std::string) { return false; };

        virtual void iterate(std::wstring path, std::function<void(s_ptr<file>)> f, bool recursive) {};
        virtual void iterate_dirs(std::wstring path, std::function<void(std::wstring)> f, bool recursive) {};

        virtual void on_change(std::wstring, std::function<void()>) {};
        // TODO:
        /*
        save_file
        get_file_info

        iterate all files|dirs

        streaming?
        */
        //  virtual file_info get_info(std::string) = 0;
        //    virtual void save_file(const std::string &, const std::string &) = 0;
};

struct file
{
        std::string load_all();
        std::shared_ptr<istream> get_new_stream()
        {
            return provider->create_stream(file_name);
        }
        file(file_provider* _provider, std::wstring _file_name) : provider(_provider), file_name(_file_name) {};
    public:
        using ptr = s_ptr<file>;
        std::wstring file_name;
        std::time_t edit_time;

        file_provider* provider;

    private:
//	std::shared_ptr<istream> input_stream;
};


class FileSystem : public Singleton<FileSystem>
{
        std::vector<file_provider::ptr> providers;
    public:
        void register_provider(file_provider::ptr provider)
        {
            providers.push_back(provider);
        }


        virtual file::ptr get_file(std::wstring);
        virtual file::ptr get_file(std::string);
        virtual bool save_data(std::wstring file_name, std::string data);

        template<class T>
        s_ptr<T> get_provider()
        {
            for (auto& p : providers)
            {
                auto r = std::dynamic_pointer_cast<T>(p);

                if (r)
                    return r;
            }

            return nullptr;
        }
        virtual void iterate(std::wstring path, std::function<void(file::ptr)> f, bool recursive);
        virtual void iterate_dirs(std::wstring path, std::function<void(std::wstring)> f, bool recursive);

        //	virtual void save_file(const std::string &, const std::string &) = 0;
};

class native_file_provider : public file_provider
{
    public:
        virtual std::string load_all(file*);

        virtual std::shared_ptr<file> get_file(std::wstring);
        virtual bool save_data(std::wstring file_name , std::string data);
        virtual void iterate(std::wstring path, std::function<void(file::ptr)> f, bool recursive);
        virtual void iterate_dirs(std::wstring path, std::function<void(std::wstring)> f, bool recursive);
        virtual void on_change(std::wstring, std::function<void()>);
        virtual std::shared_ptr<istream> create_stream(std::wstring);

		virtual ~native_file_provider()
		{
			int a;
			a = 0;
		}
};

/*
TODO:
	pack_file_provider
	resource_file_provider
	network_file_provider
*/



class resource_system
{

};

template<class _resource, class _header, class _concrete>
class resource_manager_base : public Singleton<resource_manager_base<_resource, _header, _concrete>>
{
        typedef ClassLogger<resource_system> Logger;
        friend typename _concrete;

        std::map<_header, std::shared_ptr<_resource>> resources;
        std::mutex m;
        template<class ...Args>
        std::shared_ptr<_resource> get_resource(const _header& h, Args... args)
        {
            m.lock();
            bool first = resources.find(h) == resources.end();
            auto& resource = resources[h];
            // std::shared_ptr<_resource> resource = resource_weak.lock();
            m.unlock();

            if (!resource)
            {
                if (first)
                {
                    Logger::get() << Log::LEVEL_DEBUG << " creating new resource " << typeid(_resource).name() << " with header " << NP(typeid(_header).name(), h) << Log::endl;
                    resource = _concrete::create_new(h, args...);
                    Logger::get() << Log::LEVEL_DEBUG << typeid(_resource).name() << " created" << Log::endl;
                    //   resource_weak = resource;
                }

                else
                {
                    while (!resource)
                    {
                        std::this_thread::yield();
                        // resource = resource_weak.lock();
                    }
                }
            }

            return resource;
        }

        void reload_all()
        {
            for (auto& r : resources)
            {
                auto resource = r.second;//.lock();

                if (resource)
                    resource->reload_resource();
            }
        }
};


class resource_file_depender
{
        struct depender
        {
                std::wstring file_name;
                std::time_t modify_time;

                bool need_update();
            private:
                friend class boost::serialization::access;
                template<class Archive>
                void serialize(Archive& ar, const unsigned int)
                {
                    ar& NVP(file_name)& NVP(modify_time);
                }
        };
        std::vector<depender> files;
    public:
        void add_depend(std::shared_ptr<file> _file);
        bool need_update();
        void clear();

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(files);
        }
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

        void reload_resource()
        {
            if (!file_depends.need_update())
                return;

            file_depends.clear();
            std::shared_ptr<_resource> result = _resource::load_native(header, file_depends);

            if (result)
            {
                load_succeded(result, header, file_depends);
                //   *((_resource*)this) = *result;
                swap(*static_cast<_resource*>(this), *result);
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
                result = load_from_native(header);
            else
            {
                auto stream = Serializer::get_stream(file->load_all());
                _header h;
                stream >> h;
                resource_file_depender file_depends;
                stream >> file_depends;

                if (file_depends.need_update())
                    result = load_from_native(h);
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
                Log::get().crash_error(E_FAIL, e.what());
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

template<class _resource, class _header,class _context>
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


			return load_native(header,depends,context);
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
bool loader<_resource, _header,_context>::inited = _resource::init_default_loaders();
