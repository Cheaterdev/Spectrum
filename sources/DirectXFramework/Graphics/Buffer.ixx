module;

export module Graphics:Buffer;

export import Allocators;

export import :Resource;

export import :Concepts;
export import Log;
export import :Descriptors;

export import serialization;
export import stl.core;
export import stl.memory;
export import Math;
export import :CommandList;
export import HAL;
import :HLSLDescriptors;

import d3d12;

//import :Definitions;
//using namespace HAL;
export
{


	namespace Graphics
	{


		class GPUBuffer : public Resource
		{
		protected:
			GPUBuffer();

			GPUBuffer(UINT64 count, UINT stride);


			UINT64 count;
			UINT stride;

			UINT64 size;

			HandleTable uavs;
			HandleTable srv;
			HandleTable static_uav;

			void set_data(CommandList::ptr& list, unsigned int offset, const  std::string& v);
			void set_data(unsigned int offset, const std::string& v);
		public:
			virtual ~GPUBuffer();
			using ptr = std::shared_ptr<GPUBuffer>;
			GPUBuffer(UINT64 size, HAL::ResFlags flags = HAL::ResFlags::ShaderResource, ResourceState state = ResourceState::COMMON, HeapType heap_type = HeapType::DEFAULT);

			template<class T>
			static ptr create_const_buffer()
			{
				auto res = make_shared<GPUBuffer>(sizeof(T), HAL::ResFlags::ShaderResource);
				// res->init_const_buffer();
				return res;
			}
			D3D12_CONSTANT_BUFFER_VIEW_DESC get_const_view();

			/* void init_const_buffer()
			 {
				 srv = StaticDescriptors::get().place(1);
				 D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
				 SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
				 SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
				 SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				 SRVDesc.Buffer.NumElements = count;
				 SRVDesc.Buffer.StructureByteStride = stride;
				 SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
				 Device::get().get_native_device()->CreateShaderResourceView(get_native().Get(), &SRVDesc, srv[0].cpu);
			 }
			*/


			void place_srv_buffer(Handle handle);
			void place_structured_srv(Handle h, UINT stride, unsigned int offset, unsigned int count);
			IndexBufferView get_index_buffer_view(bool is_32, unsigned int offset = 0, UINT size = 0);


			void place_raw_uav(Handle  h);

			void place_structured_uav(Handle h, GPUBuffer::ptr counter_resource, unsigned int offset = 0);

			const HandleTable& get_srv();
			const HandleTable& get_uav();
			const HandleTable& get_static_uav();
			UINT64 get_count();

			template<class T>
			void set_raw_data(std::vector<T>& v)
			{
				auto list = Device::get().get_upload_list();
				list->get_copy().update_buffer(this, 0, reinterpret_cast<const char*>(v.data()), static_cast<UINT>(v.size() * sizeof(T)));
				list->end();
				list->execute_and_wait();
			}

			template<class T>
			void set_data(const T& v)
			{
				auto list = Device::get().get_upload_list();
				list->get_copy().update_buffer(this, 0, reinterpret_cast<const char*>(&v), sizeof(T));
				list->end();
				list->execute_and_wait();
			}

			template<class T>
			void set_data(Graphics::CommandList::ptr& list, std::vector<T>& v)
			{
				list->get_copy().update_buffer(this, 0, reinterpret_cast<const char*>(v.data()), static_cast<UINT>(v.size() * sizeof(T)));
			}
			template<class T>
			void set_data(Graphics::CommandList::ptr& list, const T& v)
			{
				list->get_copy().update_buffer(this, 0, reinterpret_cast<const char*>(&v), sizeof(T));
			}
			template<class T>
			void set_data(Graphics::CommandList::ptr& list, UINT offset, const T* data, UINT size)
			{
				list->get_copy().update_buffer(this, offset, reinterpret_cast<const char*>(data), size * sizeof(T));
			}

		private:
			SERIALIZE()
			{
				IF_LOAD()
				{
					ar& NVP(count);
					ar& NVP(stride);
					ar& NVP(size);
					std::string data;
					ar& NVP(data);

					init(HAL::Resource::Buffer(size), HeapType::DEFAULT, ResourceState::COMMON);
					set_data(0, data);
				}

				else
				{
				std::string data;
				auto list = Device::get().get_upload_list();
				//	list->transition(this, ResourceState::COPY_SOURCE);
				auto task = list->get_copy().read_buffer(this, 0, get_size(), [&data](const char* mem, UINT64 size)
					{
						data.assign(mem, mem + size);
					});
				//	list->transition(this, ResourceState::COMMON);
				list->end();
				list->execute();
				task.wait();
				ar& NVP(count);
				ar& NVP(stride);
				ar& NVP(size);
				ar& NVP(data);
				}
			}


		};


		enum class counterType :int
		{
			NONE,
			SELF,
			HELP_BUFFER
		};


		template<class T>
		class StructureBuffer : public GPUBuffer
		{
			HandleTable static_raw_uav;

			StructureBuffer() = default;
			counterType counted = counterType::NONE;
			void init_views();

			HandleTable hlsl;

		public:

			HLSL::StructuredBuffer<T> structuredBuffer;
			HLSL::RWStructuredBuffer<T> rwStructuredBuffer;
			HLSL::AppendStructuredBuffer<T> appendStructuredBuffer;


			HLSL::StructuredBuffer<UINT> structuredBufferCount;

			using ptr = std::shared_ptr<StructureBuffer<T>>;
			using type = Underlying<T>;
			StructureBuffer(UINT64 count, counterType counted = counterType::NONE, HAL::ResFlags flags = HAL::ResFlags::ShaderResource, HAL::HeapType heap_type = HAL::HeapType::DEFAULT, HAL::ResourceState defaultState = HAL::ResourceState::COMMON);

			UINT get_counter_offset();

			UINT64 get_count()
			{
				return count;// get_desc().Width / sizeof(T);
			}
			using GPUBuffer::set_data;

			void set_data(Graphics::CommandList::ptr& list, unsigned int offset, std::vector<type>& v);

			void set_data(Graphics::CommandList::ptr& list, const type& v)
			{
				list->get_copy().update_buffer(this, 0, reinterpret_cast<const char*>(&v), sizeof(type));
			}

			void place_uav(Handle h);

			void place_uav(Handle h, GPUBuffer::ptr counter_resource, unsigned int offset = 0);

			Handle get_raw_uav() const
			{
				return static_raw_uav[0];
			}

			UINT get_stride()
			{
				return sizeof(type);
			}
			GPUBuffer::ptr help_buffer;
			HandleTable counted_uav;
			HandleTable counted_srv;


			void clear_counter(Graphics::CommandList::ptr& list)
			{
				list->clear_uav(counted_uav[0]);
			}

		private:

			SERIALIZE()
			{
				SAVE_PARENT(GPUBuffer);
				init_views();
			}


		};

		class ByteBuffer : public GPUBuffer
		{
			ByteBuffer() = default;

			void init_views();

		public:
			using ptr = std::shared_ptr<ByteBuffer>;
			ByteBuffer(UINT count);

			template<class T>
			void set_data(Graphics::CommandList::ptr& list, unsigned int offset, const  std::vector<T>& v)
			{
				list->get_copy().update_buffer(this, offset, reinterpret_cast<const char*>(v.data()), static_cast<UINT>(v.size() * sizeof(T)));
			}

			template<class T>
			void set_data(unsigned int offset, const  std::vector<T>& v)
			{
				auto list = Device::get().get_upload_list();
				set_data(list, offset, v);
				list->end();
				list->execute_and_wait();
			}

			void place_uav(Handle h);
			void place_srv(Handle h);

			std::function<void(Handle&)> uav()
			{
				return [this](Handle& h)
				{
					place_uav(h);
				};
			}

		private:
			SERIALIZE()
			{
				SAVE_PARENT(GPUBuffer);
				init_views();
			}


		};

		class IndexBuffer : public GPUBuffer
		{
			IndexBuffer();;
		public:
			using ptr = std::shared_ptr<IndexBuffer>;

			explicit IndexBuffer(UINT64 size);
			template<class T>
			explicit IndexBuffer(const std::vector<T>& v) : GPUBuffer(static_cast<UINT>(v.size()), sizeof(T))
			{
				set_data(0, v);
			}
			template<class T>
			void set_data(Graphics::CommandList::ptr& list, unsigned int offset, const  std::vector<T>& v)
			{
				//	list->transition(this,  ResourceState::COPY_DEST);
				list->get_copy().update_buffer(this, offset, reinterpret_cast<const char*>(v.data()), static_cast<UINT>(v.size() * sizeof(T)));
				//	list->transition(this,  ResourceState::COMMON);
			}

			template<class T>
			void set_data(unsigned int offset, const std::vector<T>& v)
			{
				auto list = Device::get().get_upload_list();
				set_data(list, offset, v);
				list->end();
				list->execute_and_wait();
			}


		private:
			SERIALIZE()
			{
				SAVE_PARENT(GPUBuffer);
			}


		};


		template<class T>
		inline void StructureBuffer<T>::init_views()
		{
			HAL::Views::ShaderResource desc = { get_hal().get(), Format::UNKNOWN, HAL::Views::ShaderResource::Buffer {0, static_cast<uint>(count), stride, false} };

			srv = StaticDescriptors::get().place(1);
			srv[0] = desc;

			hlsl = StaticDescriptors::get().place(4);

			structuredBuffer = HLSL::StructuredBuffer<T>(hlsl[0]);
			rwStructuredBuffer = HLSL::RWStructuredBuffer<T>(hlsl[1]);
			appendStructuredBuffer = HLSL::AppendStructuredBuffer<T>(hlsl[2]);
			structuredBufferCount = HLSL::StructuredBuffer<UINT>(hlsl[3]);

			structuredBuffer.create(this, 0, static_cast<uint>(count));

			if (check(get_desc().Flags & HAL::ResFlags::UnorderedAccess))
			{
				uavs = StaticDescriptors::get().place(1);
				place_uav(uavs[0]);
				static_uav = StaticDescriptors::get().place(1);
				place_uav(static_uav[0]);
				static_raw_uav = StaticDescriptors::get().place(1);

				place_raw_uav(static_raw_uav[0]);

				place_uav(rwStructuredBuffer);
				place_uav(appendStructuredBuffer);

			}

			if (counted == counterType::HELP_BUFFER)
			{
				counted_uav = StaticDescriptors::get().place(1);

				help_buffer->place_raw_uav(counted_uav[0]);

				counted_srv = StaticDescriptors::get().place(1);

				help_buffer->place_structured_srv(counted_srv[0], 4, 0, 1);

				help_buffer->place_structured_srv(structuredBufferCount, 4, 0, 1);

			}



		}
		template<class T>
		inline StructureBuffer<T>::StructureBuffer(UINT64 count, counterType counted, HAL::ResFlags flags, HeapType heap_type, ResourceState defaultState) : GPUBuffer(counted == counterType::SELF ? (Math::AlignUp(count * sizeof(type), D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT) + (counted == counterType::SELF) * sizeof(UINT)) : (count * sizeof(type)), flags, defaultState, heap_type)
		{
			stride = sizeof(type);
			this->count = count;
			this->counted = counted;


			if (counted == counterType::HELP_BUFFER)
				help_buffer = std::make_shared<GPUBuffer>(4, flags);

			//	if (count>0)
			init_views();


		}
		template<class T>
		inline UINT StructureBuffer<T>::get_counter_offset()
		{
			assert(counted == counterType::SELF && "needs to be counted");
			return size - sizeof(UINT);
		}

		template<class T>
		inline void StructureBuffer<T>::set_data(Graphics::CommandList::ptr& list, unsigned int offset, std::vector<type>& v)
		{
			list->get_copy().update_buffer(this, offset, reinterpret_cast<const char*>(v.data()), static_cast<UINT>(v.size() * sizeof(type)));
		}

		template<class T>
		inline void StructureBuffer<T>::place_uav(Handle  h)
		{
			HAL::Resource* counter_resource = get_hal().get();
			uint64 counter_offset = 0;

			if (counted == counterType::HELP_BUFFER) counter_resource = help_buffer->get_hal().get();
			if (counted == counterType::SELF)	counter_offset = std::max(0, (int)size - (int)sizeof(UINT));
			auto b = HAL::Views::UnorderedAccess::Buffer{ 0u, static_cast<uint>(count),  static_cast<uint>(stride), false, counter_offset, counter_resource };
			h = HAL::Views::UnorderedAccess{ get_hal().get(), Format::UNKNOWN, b };
		}

		template<class T>
		inline void StructureBuffer<T>::place_uav(Handle  h, GPUBuffer::ptr counter_resource, unsigned int offset)
		{
			auto b = HAL::Views::UnorderedAccess::Buffer{ offset, static_cast<uint>(count),  static_cast<uint>(stride), false, 0,  nullptr };
			h = HAL::Views::UnorderedAccess{ get_hal().get(), Format::UNKNOWN, b };
		}




	}

}









namespace Graphics
{

	GPUBuffer::GPUBuffer(UINT64 _size, HAL::ResFlags flags, ResourceState state, HeapType heap_type) : size(Math::AlignUp(_size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)),
		count(Math::AlignUp(_size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)),
		stride(1), Resource({ HAL::BufferDesc{ Math::AlignUp(_size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT) }, flags
			}, heap_type, state)
	{
	}

	D3D12_CONSTANT_BUFFER_VIEW_DESC GPUBuffer::get_const_view()
	{
		return{ get_gpu_address(), static_cast<UINT>(size) };
	}

	GPUBuffer::GPUBuffer()
	{
	}

	GPUBuffer::GPUBuffer(UINT64 count, UINT stride) : count(count),
		stride(stride), Resource({ HAL::BufferDesc{ std::max(static_cast<UINT64>(4), count * stride) }, HAL::ResFlags::ShaderResource }, HeapType::DEFAULT, ResourceState::COMMON)
	{
		size = count * stride;
	}

	void GPUBuffer::set_data(unsigned int offset, const std::string& v)
	{
		auto list = Device::get().get_upload_list();
		set_data(list, offset, v);
		list->end();
		list->execute_and_wait();
	}

	GPUBuffer::~GPUBuffer()
	{
		size = 0;
	}

	void GPUBuffer::place_raw_uav(Handle h)
	{
		auto b = HAL::Views::UnorderedAccess::Buffer{ 0u, static_cast<UINT>(get_desc().as_buffer().SizeInBytes / 4),  0u, true, 0, nullptr };
		h = HAL::Views::UnorderedAccess{ get_hal().get(), Format::R32_TYPELESS, b };
	}

	void GPUBuffer::set_data(Graphics::CommandList::ptr& list, unsigned int offset, const std::string& v)
	{
		list->get_copy().update_buffer(this, offset, reinterpret_cast<const char*>(v.data()), static_cast<UINT>(v.size() * sizeof(char)));
	}

	void GPUBuffer::place_srv_buffer(Handle  handle)
	{
		HAL::Views::ShaderResource desc = { get_hal().get(), Format::R32G32B32A32_FLOAT, HAL::Views::ShaderResource::Buffer {0, static_cast<UINT>(count / sizeof(vec4)), 0, false} };

		handle = desc;
	}

	void GPUBuffer::place_structured_uav(Handle  h, GPUBuffer::ptr counter_resource, unsigned int offset)
	{
		auto b = HAL::Views::UnorderedAccess::Buffer{ offset, static_cast<UINT>(count),  stride, false, 0, nullptr };
		h = HAL::Views::UnorderedAccess{ get_hal().get(), Format::UNKNOWN, b };
	}

	void GPUBuffer::place_structured_srv(Handle handle, UINT stride, unsigned int offset, unsigned int count)
	{
		HAL::Views::ShaderResource desc = { get_hal().get(), Format::R32G32B32A32_FLOAT, HAL::Views::ShaderResource::Buffer {offset,count, 0, false} };
		handle = desc;
	}

	const HandleTable& GPUBuffer::get_srv()
	{
		return srv;
	}

	const HandleTable& GPUBuffer::get_uav()
	{
		return uavs;
	}
	const HandleTable& GPUBuffer::get_static_uav()
	{
		return static_uav;
	}
	UINT64 GPUBuffer::get_count()
	{
		return count;
	}
	ByteBuffer::ByteBuffer(UINT count) : GPUBuffer(count, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess)
	{
		init_views();
	}

	void ByteBuffer::init_views()
	{
		srv = StaticDescriptors::get().place(1);
		place_srv(srv[0]);
		uavs = StaticDescriptors::get().place(1);
		place_uav(uavs[0]);
		static_uav = StaticDescriptors::get().place(1);
		place_uav(static_uav[0]);
	}


	void ByteBuffer::place_uav(Handle h)
	{

		auto b = HAL::Views::UnorderedAccess::Buffer{ 0, static_cast<UINT>(count / 4),  0, true, 0, nullptr };
		h = HAL::Views::UnorderedAccess{ get_hal().get(), Format::R32_TYPELESS, b };

	}

	void ByteBuffer::place_srv(Handle h)
	{
		HAL::Views::ShaderResource desc = { get_hal().get(), Format::R32G32B32A32_FLOAT, HAL::Views::ShaderResource::Buffer {0,static_cast<UINT>(count / 4), 0, true} };
		h = desc;
	}
	IndexBuffer::IndexBuffer(UINT64 size) : GPUBuffer(size)
	{
	}

	IndexBuffer::IndexBuffer()
	{
	}

	IndexBufferView GPUBuffer::get_index_buffer_view(bool is_32, unsigned int offset /*= 0*/, UINT size /*= 0*/)
	{
		IndexBufferView view;
		view.view.BufferLocation = get_gpu_address() + offset;
		view.view.Format = is_32 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
		view.view.SizeInBytes = static_cast<UINT>(size ? size : this->size);
		view.resource = this;
		return view;
	}

}
