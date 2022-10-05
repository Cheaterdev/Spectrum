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
			HandleTable hlsl;


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
			IndexBufferView get_index_buffer_view(unsigned int offset = 0, UINT size = 0);


			UINT64 get_count();

			template<class T>
			void set_raw_data(const std::vector<T>& v)
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
				auto task = list->get_copy().read_buffer(this, 0, get_size(), [&data](const char* mem, UINT64 size)
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
			HLSL::RWStructuredBuffer<UINT> rwStructuredBufferCount;


			HLSL::RWByteAddressBuffer rwByteAddressBuffer;
			HLSL::RWByteAddressBuffer rwByteAddressBufferCount;


			using ptr = std::shared_ptr<StructureBuffer<T>>;
			using type = Underlying<T>;
			StructureBuffer(UINT64 count, counterType counted = counterType::NONE, HAL::ResFlags flags = HAL::ResFlags::ShaderResource, HAL::HeapType heap_type = HAL::HeapType::DEFAULT, HAL::ResourceState defaultState = HAL::ResourceState::COMMON);



			explicit StructureBuffer(const std::vector<T>& v) : StructureBuffer(v.size())
			{
				GPUBuffer::set_raw_data(v);
			}


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

			UINT get_stride()
			{
				return sizeof(type);
			}
			GPUBuffer::ptr help_buffer;


			void clear_counter(Graphics::CommandList::ptr& list)
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
			hlsl = StaticDescriptors::get().place(7);

			structuredBuffer = HLSL::StructuredBuffer<T>(hlsl[0]);
			rwStructuredBuffer = HLSL::RWStructuredBuffer<T>(hlsl[1]);
			appendStructuredBuffer = HLSL::AppendStructuredBuffer<T>(hlsl[2]);

			structuredBufferCount = HLSL::StructuredBuffer<UINT>(hlsl[3]);
			rwStructuredBufferCount = HLSL::RWStructuredBuffer<UINT>(hlsl[4]);

			rwByteAddressBuffer = HLSL::RWByteAddressBuffer(hlsl[5]);
			rwByteAddressBufferCount = HLSL::RWByteAddressBuffer(hlsl[6]);

			Graphics::Resource* counter_resource = this;
			uint64 counter_offset = 0;

			if (counted == counterType::HELP_BUFFER) counter_resource = help_buffer.get();
			if (counted == counterType::SELF)	counter_offset = std::max(0, (int)size - (int)sizeof(UINT));

			if (check(get_desc().Flags & HAL::ResFlags::ShaderResource))
			{
				structuredBuffer.create(this, 0, static_cast<uint>(count));
				structuredBufferCount.create(counter_resource, counter_offset / 4, 1);
			}

			if (check(get_desc().Flags & HAL::ResFlags::UnorderedAccess))
			{
				rwStructuredBuffer.create(this, 0, count);
				appendStructuredBuffer.create(counter_resource, counter_offset, this, 0, count);
				rwStructuredBufferCount.create(counter_resource, counter_offset / 4, 1);

				rwByteAddressBuffer.create(this, 0, get_desc().as_buffer().SizeInBytes / 4);
				rwByteAddressBufferCount.create(counter_resource, counter_offset / 4, 1);
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


	void GPUBuffer::set_data(Graphics::CommandList::ptr& list, unsigned int offset, const std::string& v)
	{
		list->get_copy().update_buffer(this, offset, reinterpret_cast<const char*>(v.data()), static_cast<UINT>(v.size() * sizeof(char)));
	}


	UINT64 GPUBuffer::get_count()
	{
		return count;
	}

	IndexBufferView GPUBuffer::get_index_buffer_view(unsigned int offset /*= 0*/, UINT size /*= 0*/)
	{
		IndexBufferView view;
		view.view.BufferLocation = get_gpu_address() + offset;
		view.view.Format = DXGI_FORMAT_R32_UINT;// is_32 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
		view.view.SizeInBytes = static_cast<UINT>(size ? size : this->size);
		view.resource = this;
		return view;
	}

}
