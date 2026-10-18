export module HAL:Shader;

import :ShaderCompiler;
import Core;
import :Enums;
import :Slots;
import :Utils;   // get_backend_name() for per-backend shader cache
export std::optional<SlotID> get_slot(std::string_view slot_name);   // autogen.ixx exports it too; every declaration must agree (Clang)
export
{

	// Compiled shaders can travel inside the asset that uses them (a material's pipelines): saving with
	// `enabled()` writes each shader's cache file next to its header, loading puts it back in the shader cache
	// before the shader is looked up, so a device with no compiler never needs the source. Off unless an
	// offline tool turns it on.
	struct ShaderEmbedding
	{
		static bool& enabled() { static bool v = false; return v; }

		static std::filesystem::path cache_file(const HAL::shader_header& header)
		{
			return std::filesystem::path("cache") / get_backend_name() / (Hasher::hash(header) + ".bin");
		}

		static std::string read(const HAL::shader_header& header)
		{
			std::ifstream file(cache_file(header), std::ios::binary);
			if (!file.is_open())
				return {};
			return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
		}

		static void restore(const HAL::shader_header& header, const std::string& blob)
		{
			if (blob.empty())
				return;

			// Assets load on several threads and share shaders: the file must appear complete or not at all
			// (a reader that finds it half written fails), and one thread writes it.
			static std::mutex mutex;
			std::lock_guard<std::mutex> lock(mutex);

			const auto path = cache_file(header);
			if (std::filesystem::exists(path))
				return;
			std::error_code ec;
			std::filesystem::create_directories(path.parent_path(), ec);

			auto partial = path;
			partial += ".partial";
			{
				std::ofstream file(partial, std::ios::binary);
				file.write(blob.data(), (std::streamsize)blob.size());
			}
			std::filesystem::rename(partial, path, ec);
		}
	};

	template<class T>
	class shader_with_id
	{
		std::shared_ptr<T> shader;
	public:
		using ShaderType = T;
		shader_with_id() = default;
		shader_with_id(const std::shared_ptr<T>& o) :shader(o)
		{

		}

		shader_with_id(T* o) :shader(o)
		{

		}

		shader_with_id(std::nullptr_t) :shader(nullptr)
		{

		}

		T* operator->()
		{
			return shader.get();
		}

		T& operator*()
		{
			return *shader;
		}


		T* operator->() const
		{
			return shader.get();
		}

		T& operator*() const
		{
			return *shader;
		}

		operator bool() const
		{
			return !!shader;
		}

		bool operator==(const shader_with_id<T>& r) const
		{
			size_t my = shader ? shader->id : -5;
			size_t other = r.shader ? r.shader->id : -5;

			return my == other;
		}
		std::strong_ordering  operator<=>(const  shader_with_id<T>& r)  const
		{
			size_t my = shader ? shader->id : -5;
			size_t other = r.shader ? r.shader->id : -5;

			return my <=> other;
		}
	private:
		SERIALIZE()
		{

			if constexpr (Archive::is_saving::value)
				{

		
						bool has_header = !!shader;
						ar& NVP(has_header);

						if (has_header)
						{
							ar& NVP(shader->get_header());

							std::string compiled = ShaderEmbedding::enabled() ? ShaderEmbedding::read(shader->get_header()) : std::string();
							ar& NVP(compiled);
						}
				}
				else
				{
	
				
							bool has_header;
						ar& NVP(has_header);

						if (has_header)
						{
							HAL::shader_header header;
							ar& NVP(header);

							std::string compiled;
							ar& NVP(compiled);
							ShaderEmbedding::restore(header, compiled);

							shader = ShaderType::get_resource(header);
						}

				
				}
		}
	};
	namespace HAL
	{
		//class PipelineStateBase;


		template<class _shader_type>
		class Shader : public resource_manager<_shader_type, shader_header>
		{
			using Manager = resource_manager<_shader_type, shader_header>;

			friend Manager;
		protected:
			static const char* compile_code;
			static const char* compile_code_dxil;

			unsigned int hash;
			unsigned int blob_hash;

			//   DX11_ClassLinkage class_linkage;
			//   std::vector<ID3D11ClassInstance*>    class_instances;
			virtual void compile() {};


			void operator=(const Shader& r)
			{
				Manager::operator=(r);
				// reflection = r.reflection;
				blob = r.blob;
				hash = r.hash;
				slots_usage = r.slots_usage;
				on_change();
			}


			void own_id()
			{
				blob_hash = crc32(blob.blob);
				this->id = shader_ids[blob_hash];
			}
		public:
			// Per-backend shader cache: a SPIR-V (Vulkan) blob and a DXIL (D3D12)
			// blob hash to the same source but must never share a cache slot.
			// → cache/<backend>/<hash>.bin
			static std::string cache_subfolder() { return get_backend_name(); }

			static Cache<unsigned int, size_t> shader_ids;
					CompiledShader blob;
		
			UsedSlots slots_usage;
			Events::Event<> on_change;
			const unsigned int& get_hash() const
			{
				return hash;
			}

			size_t id;

			binary& get_blob()
			{
				return blob.blob;
			}

			const HAL::shader_header& get_header() const
			{
				return Manager::header;
			}

			static std::shared_ptr<_shader_type> load_native(const HAL::shader_header& header, resource_file_depender& depender)
			{

				std::optional<CompiledShader> res_blob;


				while (!res_blob)
				{
					depender.clear();


					if (header.contains_text)
					{
						HAL::shader_include In("shaders/", depender);
						res_blob = ShaderCompiler::get().Compile_Shader(header.file_name, header.macros, compile_code_dxil, header.entry_point, header.flags, &In);
					}
					else
					{
						HAL::shader_include In(header.file_name, depender);
						res_blob = ShaderCompiler::get().Compile_Shader_File(header.file_name, header.macros, compile_code_dxil, header.entry_point,header.flags,  &In);
					}
					// The retry loop exists for editing a broken shader live. With no compiler
					// a failure is permanent, so give back an empty shader instead of spinning.
					if (!res_blob && !ShaderCompiler::can_compile)
					{
						std::string macros;
						for (auto& m : header.macros)
							macros += " " + m.name + "=" + m.value;
						Log::get() << Log::LEVEL_ERROR << "missing precompiled shader cache/" << get_backend_name() << "/" << Hasher::hash(header)
						           << ".bin: " << header.file_name << " " << header.entry_point << macros << Log::endl;

						auto empty = std::make_shared<_shader_type>();
						empty->header = header;
						return empty;
					}
				}

				auto result = std::make_shared<_shader_type>();
				result->header = header;
				result->blob = std::move(*res_blob);
				result->compile();
				result->own_id();
				result->hash = crc32(result->blob.blob);
				// Derived from DXC/D3D12 reflection on the compiled entry point(s),
				// not from which files got #included -- a shared .hlsl with multiple
				// entry points can't be told apart by file dependencies alone.
				for (auto& f : result->blob.functions)
					result->slots_usage.merge(f.slots);
				result->init();
				return result;
			}
		private:
			SERIALIZE()
			{	SAVE_PARENT(Manager);

				ar& NVP(blob);
				ar& NVP(hash);
				ar& NVP(slots_usage);

				IF_LOAD()
				{
					own_id();
					compile();
				}
			}
		};

		template<class _shader_type>
		Cache<unsigned int, size_t> Shader<_shader_type>::shader_ids([](unsigned int)
			{
				return Shader<_shader_type>::shader_ids.size();
			});







		class pixel_shader : public Shader<pixel_shader>
		{
			friend class Shader<pixel_shader>;
			void init();
		public:

			using ptr = shader_with_id<pixel_shader>;

			static const ptr null;
		};


		class geometry_shader : public Shader<geometry_shader>
		{
			friend class Shader<geometry_shader>;
			void init();

		public:

			using ptr = shader_with_id<geometry_shader>;

			static const ptr null;
		protected:
			friend class Context;


		};



		class vertex_shader : public Shader<vertex_shader>
		{
			friend class Shader<vertex_shader>;
			void init();

		public:

			using ptr = shader_with_id<vertex_shader>;

			static const ptr null;


		};



		class hull_shader : public Shader<hull_shader>
		{
			friend class Shader<hull_shader>;
			void init();
		public:
			using ptr = shader_with_id<hull_shader>;
			static const ptr null;
		};




		class domain_shader : public Shader<domain_shader>
		{
			friend class Shader<domain_shader>;
			void init();
		public:

			using ptr = shader_with_id<domain_shader>;

			static const ptr null;
		};

		class compute_shader : public Shader<compute_shader>
		{
			friend class Shader<compute_shader>;
			void init();
		public:

			using ptr = shader_with_id<compute_shader>;

			static const ptr null;
		};


		class mesh_shader : public Shader<mesh_shader>
		{
			friend class Shader<mesh_shader>;
			void init();
		public:

			using ptr = shader_with_id<mesh_shader>;

			static const ptr null;
		};

		class amplification_shader : public Shader<amplification_shader>
		{
			friend class Shader<amplification_shader>;
			void init();
		public:

			using ptr = shader_with_id<amplification_shader>;

			static const ptr null;
		};


		class library_shader : public Shader<library_shader>
		{
			friend class Shader<library_shader>;

			void init();
		public:

			using ptr = shader_with_id<library_shader>;

			static const ptr null;


		};

	}



}