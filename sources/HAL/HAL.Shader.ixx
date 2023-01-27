export module HAL:Shader;

import :ShaderCompiler;
import Core;
import :Enums;
import :Slots;
std::optional<SlotID> get_slot(std::string_view slot_name);
export
{

	template<class T>
	class shader_with_id
	{
		std::shared_ptr<T> shader;
	public:

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
			ar& NVP(shader);
		}

	};
	namespace HAL
	{
		//class PipelineStateBase;


		template<class _shader_type>
		class Shader : public resource_manager<_shader_type, shader_header>
		{
			friend class resource_manager<_shader_type, shader_header>;
		protected:
			binary blob;
			static const char* compile_code;
			static const char* compile_code_dxil;

			unsigned int hash;
			unsigned int blob_hash;

			//   DX11_ClassLinkage class_linkage;
			//   std::vector<ID3D11ClassInstance*>    class_instances;
			virtual void compile() {};


			void operator=(const Shader& r)
			{
				resource_manager<_shader_type, shader_header>::operator=(r);
				// reflection = r.reflection;
				blob = r.blob;
				hash = r.hash;
				slots_usage = r.slots_usage;
				on_change();
			}


			void own_id()
			{
				blob_hash = crc32(blob);
				this->id = shader_ids[blob_hash];
			}
		public:
			static Cache<unsigned int, size_t> shader_ids;

			UsedSlots slots_usage;
			Events::Event<void> on_change;
			const unsigned int& get_hash() const
			{
				return hash;
			}

			size_t id;

			binary& get_blob()
			{
				return blob;
			}

			const HAL::shader_header& get_header() const
			{
				return resource_manager<_shader_type, HAL::shader_header>::header;
			}

			static shader_with_id<_shader_type> create_from_memory(std::string data, std::string func_name, UINT flags, std::vector<HAL::shader_macro> macros = {})
			{
				// auto t = CounterManager::get().start_count<_shader_type>();
				resource_file_depender depender;
				HAL::shader_include In("shaders/", depender);

				std::optional<binary> res_blob;

				while (!res_blob)
				{
					depender.clear();
					res_blob = ShaderCompiler::get().Compile_Shader(data, macros, compile_code_dxil, func_name, &In);
				}
				auto result = std::make_shared<_shader_type>();


				result->blob = std::move(*res_blob);
				result->own_id();
				result->compile();
				result->hash = crc32(result->blob);
				result->slots_usage.merge(depender);

			//	assert(!result->slots_usage.empty());
				return result;
			}

			static std::shared_ptr<_shader_type> load_native(const HAL::shader_header& header, resource_file_depender& depender)
			{

				std::optional<binary> res_blob;


				while (!res_blob)
				{
					depender.clear();


					if (header.contains_text)
					{
						HAL::shader_include In("shaders/", depender);
						res_blob = ShaderCompiler::get().Compile_Shader(header.file_name, header.macros, compile_code_dxil, header.entry_point, &In);
					}
					else
					{
						HAL::shader_include In(header.file_name, depender);
						res_blob = ShaderCompiler::get().Compile_Shader_File(header.file_name, header.macros, compile_code_dxil, header.entry_point, &In);
					}
				}

				auto result = std::make_shared<_shader_type>();
				result->header = header;
				result->blob = std::move(*res_blob);
				result->compile();
				result->own_id();
				result->hash = crc32(result->blob);
				result->slots_usage.merge(depender);

				return result;
			}
		private:
			SERIALIZE()
			{
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
		public:

			using ptr = shader_with_id<pixel_shader>;

			static const ptr null;
		};


		class geometry_shader : public Shader<geometry_shader>
		{
			friend class Shader<geometry_shader>;

		public:

			using ptr = shader_with_id<geometry_shader>;

			static const ptr null;
		protected:
			friend class Context;


		};



		class vertex_shader : public Shader<vertex_shader>
		{
			friend class Shader<vertex_shader>;

		public:

			using ptr = shader_with_id<vertex_shader>;

			static const ptr null;


		};



		class hull_shader : public Shader<hull_shader>
		{
			friend class Shader<hull_shader>;
		public:
			using ptr = shader_with_id<hull_shader>;
			static const ptr null;
		};




		class domain_shader : public Shader<domain_shader>
		{
			friend class Shader<domain_shader>;
		public:

			using ptr = shader_with_id<domain_shader>;

			static const ptr null;
		};

		class compute_shader : public Shader<compute_shader>
		{
			friend class Shader<compute_shader>;
		public:

			using ptr = shader_with_id<compute_shader>;

			static const ptr null;
		};


		class mesh_shader : public Shader<mesh_shader>
		{
			friend class Shader<mesh_shader>;
		public:

			using ptr = shader_with_id<mesh_shader>;

			static const ptr null;
		};

		class amplification_shader : public Shader<amplification_shader>
		{
			friend class Shader<amplification_shader>;
		public:

			using ptr = shader_with_id<amplification_shader>;

			static const ptr null;
		};


		class library_shader : public Shader<library_shader>
		{
			friend class Shader<library_shader>;


		public:

			using ptr = shader_with_id<library_shader>;

			static const ptr null;


		};

	}



}