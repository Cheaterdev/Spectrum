export module HAL:Buffer;

export import :Types;
export import :Concepts;
import Core;

import :Resource;

import :ResourceStates;

import :DescriptorHeap;
import :CommandList;
import :HLSL;

export
{


	namespace HAL
	{


		class GPUBuffer
		{
		protected:
		/*	GPUBuffer();*/
		
			GPUBuffer(UINT64 count, UINT stride);


			UINT64 count;
			UINT stride;

			UINT64 size;

			void set_data(HAL::CommandList::ptr& list, unsigned int offset, const  std::string& v);
			void set_data(unsigned int offset, const std::string& v);
		public:
			virtual ~GPUBuffer();
			using ptr = std::shared_ptr<GPUBuffer>;
			GPUBuffer(UINT64 size, HAL::ResFlags flags = HAL::ResFlags::ShaderResource, HAL::ResourceState state = HAL::ResourceState::COMMON, HAL::HeapType heap_type = HAL::HeapType::DEFAULT);
	HAL::Resource::ptr resource;

			template<class T>
			static ptr create_const_buffer()
			{
				auto res = make_shared<GPUBuffer>(sizeof(T), HAL::ResFlags::ShaderResource);
				// res->init_const_buffer();
				return res;
			}

			HAL::Views::IndexBuffer get_index_buffer_view(unsigned int offset = 0, UINT size = 0);


			UINT64 get_count();

			template<class T>
			void set_raw_data(const std::vector<T>& v)
			{
				auto list = (HAL::Device::get().get_upload_list());
				list->get_copy().update_buffer(resource, 0, reinterpret_cast<const char*>(v.data()), static_cast<UINT>(v.size() * sizeof(T)));
				list->execute_and_wait();
			}

			template<class T>
			void set_data(const T& v)
			{
				auto list = (HAL::Device::get().get_upload_list());
				list->get_copy().update_buffer(resource, 0, reinterpret_cast<const char*>(&v), sizeof(T));
				list->execute_and_wait();
			}

			template<class T>
			void set_data(HAL::CommandList::ptr& list, std::vector<T>& v)
			{
				list->get_copy().update_buffer(resource, 0, reinterpret_cast<const char*>(v.data()), static_cast<UINT>(v.size() * sizeof(T)));
			}
			template<class T>
			void set_data(HAL::CommandList::ptr& list, const T& v)
			{
				list->get_copy().update_buffer(resource, 0, reinterpret_cast<const char*>(&v), sizeof(T));
			}
			template<class T>
			void set_data(HAL::CommandList::ptr& list, UINT offset, const T* data, UINT size)
			{
				list->get_copy().update_buffer(resource, offset, reinterpret_cast<const char*>(data), size * sizeof(T));
			}

				ResourceAddress get_resource_address() const
			{
				return resource->get_resource_address();
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

					init(HAL::Resource::Buffer(size), HAL::HeapType::DEFAULT, HAL::ResourceState::COMMON);
					set_data(0, data);
				}

				else
				{
				std::string data;
				auto list = (HAL::Device::get().get_upload_list());
				auto task = list->get_copy().read_buffer(resource, 0, resource->get_size(), [&data](const char* mem, UINT64 size)
					{
						data.assign(mem, mem + size);
					});
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

			StructureBuffer() = default;
			counterType counted = counterType::NONE;
			void init_views();

		public:

			HLSL::StructuredBuffer<T> structuredBuffer;
			HLSL::RWStructuredBuffer<T> rwStructuredBuffer;
			HLSL::AppendStructuredBuffer<T> appendStructuredBuffer;


			HLSL::StructuredBuffer<UINT> structuredBufferCount;
			HLSL::RWStructuredBuffer<UINT> rwStructuredBufferCount;


			HLSL::RWByteAddressBuffer rwByteAddressBuffer;
			HLSL::RWByteAddressBuffer rwByteAddressBufferCount;


			using ptr = std::shared_ptr<StructureBuffer<T>>;
			using type = Underlying<T>;
			StructureBuffer(UINT64 count, counterType counted = counterType::NONE, HAL::ResFlags flags = HAL::ResFlags::ShaderResource, HAL::HeapType heap_type = HAL::HeapType::DEFAULT, HAL::ResourceState defaultState = HAL::ResourceState::COMMON);


			static StructureBuffer::ptr make_buffer(const std::vector<T>& v)
			{
				auto result = std::make_shared<StructureBuffer>(v.size());
				result->set_raw_data(v);
				return result;
			}


			UINT get_counter_offset();

			UINT64 get_count()
			{
				return count;// get_desc().Width / sizeof(T);
			}
			using GPUBuffer::set_data;

			void set_data(HAL::CommandList::ptr& list, unsigned int offset, std::vector<type>& v);

			void set_data(HAL::CommandList::ptr& list, const type& v)
			{
				list->get_copy().update_buffer(this, 0, reinterpret_cast<const char*>(&v), sizeof(type));
			}

			UINT get_stride()
			{
				return sizeof(type);
			}
			GPUBuffer::ptr help_buffer;


			void clear_counter(HAL::CommandList::ptr& list)
			{
				list->clear_uav(rwByteAddressBufferCount);
			}

		private:

			SERIALIZE()
			{
				SAVE_PARENT(GPUBuffer);
				init_views();
			}


		};

		using IndexBuffer = StructureBuffer<unsigned int>;

		template<class T>
		inline void StructureBuffer<T>::init_views()
		{
			auto hlsl = HAL::Device::get().get_static_gpu_data().alloc_descriptor(7, DescriptorHeapIndex{ HAL::DescriptorHeapType::CBV_SRV_UAV, HAL::DescriptorHeapFlags::ShaderVisible });

			structuredBuffer = HLSL::StructuredBuffer<T>(hlsl[0]);
			rwStructuredBuffer = HLSL::RWStructuredBuffer<T>(hlsl[1]);
			appendStructuredBuffer = HLSL::AppendStructuredBuffer<T>(hlsl[2]);

			structuredBufferCount = HLSL::StructuredBuffer<UINT>(hlsl[3]);
			rwStructuredBufferCount = HLSL::RWStructuredBuffer<UINT>(hlsl[4]);

			rwByteAddressBuffer = HLSL::RWByteAddressBuffer(hlsl[5]);
			rwByteAddressBufferCount = HLSL::RWByteAddressBuffer(hlsl[6]);

			HAL::Resource::ptr counter_resource = resource;
			uint64 counter_offset = 0;

			if (counted == counterType::HELP_BUFFER) counter_resource = help_buffer->resource;
			if (counted == counterType::SELF)	counter_offset = std::max(0, (int)size - (int)sizeof(UINT));

			if (check(resource->get_desc().Flags & HAL::ResFlags::ShaderResource))
			{
				structuredBuffer.create(resource, 0, static_cast<uint>(count));
				structuredBufferCount.create(counter_resource, counter_offset / 4, 1);
			}

			if (check(resource->get_desc().Flags & HAL::ResFlags::UnorderedAccess))
			{
				rwStructuredBuffer.create(resource, 0, count);
				appendStructuredBuffer.create(counter_resource, counter_offset, resource, 0, count);
				rwStructuredBufferCount.create(counter_resource, counter_offset / 4, 1);

				rwByteAddressBuffer.create(resource, 0, resource->get_desc().as_buffer().SizeInBytes / 4);
				rwByteAddressBufferCount.create(counter_resource, counter_offset / 4, 1);
			}
		}

		template<class T>
		inline StructureBuffer<T>::StructureBuffer(UINT64 count, counterType counted, HAL::ResFlags flags, HAL::HeapType heap_type, HAL::ResourceState defaultState) : GPUBuffer(counted == counterType::SELF ? (Math::AlignUp(count * sizeof(type), D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT) + (counted == counterType::SELF) * sizeof(UINT)) : (count * sizeof(type)), flags, defaultState, heap_type)
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
		inline void StructureBuffer<T>::set_data(HAL::CommandList::ptr& list, unsigned int offset, std::vector<type>& v)
		{
			list->get_copy().update_buffer(resource, offset, reinterpret_cast<const char*>(v.data()), static_cast<UINT>(v.size() * sizeof(type)));
		}




	}

}









namespace HAL
{

	GPUBuffer::GPUBuffer(UINT64 _size, HAL::ResFlags flags, HAL::ResourceState state, HAL::HeapType heap_type) : size(Math::AlignUp(_size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)),
		count(Math::AlignUp(_size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)),
		stride(1)
	{

		resource = std::make_shared<HAL::Resource>( HAL::ResourceDesc::Buffer(count, flags), heap_type, state);
	}


	GPUBuffer::GPUBuffer(UINT64 count, UINT stride) : count(count),stride(stride)
	{
		size = count * stride;

		resource = std::make_shared<HAL::Resource>( HAL::ResourceDesc::Buffer(std::max(static_cast<UINT64>(4), count * stride), HAL::ResFlags::ShaderResource), HAL::HeapType::DEFAULT, HAL::ResourceState::COMMON);
	}

	void GPUBuffer::set_data(unsigned int offset, const std::string& v)
	{
		auto list = (HAL::Device::get().get_upload_list());
		set_data(list, offset, v);
		list->execute_and_wait();
	}

	GPUBuffer::~GPUBuffer()
	{
		size = 0;
	}


	void GPUBuffer::set_data(HAL::CommandList::ptr& list, unsigned int offset, const std::string& v)
	{
		list->get_copy().update_buffer(resource, offset, reinterpret_cast<const char*>(v.data()), static_cast<UINT>(v.size() * sizeof(char)));
	}


	UINT64 GPUBuffer::get_count()
	{
		return count;
	}

	HAL::Views::IndexBuffer GPUBuffer::get_index_buffer_view(unsigned int offset /*= 0*/, UINT size /*= 0*/)
	{
		HAL::Views::IndexBuffer view;
		view.OffsetInBytes = offset;
		view.Format = HAL::Format::R32_UINT;// is_32 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
		view.SizeInBytes = static_cast<UINT>(size ? size : this->size);
		view.Resource = resource;
		return view;
	}

}
