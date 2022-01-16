module;
#include "dx12_types.h"
export module Buffer;

export import Allocators;

export import Device;
export import Resource;

export import Concepts;
export import Log;
export import Descriptors;

export import serialization;
export import stl.core;
export import stl.memory;
export import Constants;
export import Vectors;
export import Matrices;
export import CommandList;
export import HAL.Types;

using namespace HAL;
export
{


	namespace DX12
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
			GPUBuffer(UINT64 size, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE, ResourceState state = ResourceState::COMMON, HeapType heap_type = HeapType::DEFAULT);

			template<class T>
			static ptr create_const_buffer()
			{
				auto res = make_shared<GPUBuffer>(sizeof(T), D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE);
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
			void set_data(DX12::CommandList::ptr& list, std::vector<T>& v)
			{
				list->get_copy().update_buffer(this, 0, reinterpret_cast<const char*>(v.data()), static_cast<UINT>(v.size() * sizeof(T)));
			}
			template<class T>
			void set_data(DX12::CommandList::ptr& list, const T& v)
			{
				list->get_copy().update_buffer(this, 0, reinterpret_cast<const char*>(&v), sizeof(T));
			}
			template<class T>
			void set_data(DX12::CommandList::ptr& list, UINT offset, const T* data, UINT size)
			{
				list->get_copy().update_buffer(this, offset, reinterpret_cast<const char*>(data), size * sizeof(T));
			}

		private:
			SERIALIZE()
			{
				if (Archive::is_loading::value)
				{
					ar& NVP(count);
					ar& NVP(stride);
					ar& NVP(size);
					std::string data;
					ar& NVP(data);

					if (debug) {
						UINT32* ptr = (UINT32*)data.data();

						size_t count3 = data.size() / (sizeof(UINT32) * 3);

						std::set<UINT32> indices[3];
						for (size_t i = 0; i < count3 * 3; i++)
						{
							indices[i / (count3)].emplace(ptr[i]);
						}


					}
					init(CD3DX12_RESOURCE_DESC::Buffer(size), HeapType::DEFAULT, ResourceState::COMMON);
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

		class UploadBuffer : public Resource
		{

		public:

			using ptr = std::shared_ptr<UploadBuffer>;
			UploadBuffer(UINT64 count);
			virtual ~UploadBuffer();
			char* get_data();




		};


		class CPUBuffer : public Resource
		{
			int stride;

		public:
			CPUBuffer(UINT64 count, int stride);

			bool mapped = false;
			template<class T = char>
			T* map(int from, int to)
			{
				T* result = nullptr;
				mapped = true;
				D3D12_RANGE Range;
				Range.Begin = from * stride;
				Range.End = to * stride;
				tracked_info->m_Resource->Map(0, &Range, reinterpret_cast<void**>(&result));
				return result;
			}


			void unmap();

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
			StructureBuffer(UINT64 count, counterType counted = counterType::NONE, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE, HAL::HeapType heap_type = HAL::HeapType::DEFAULT, HAL::ResourceState defaultState = HAL::ResourceState::COMMON);

			UINT get_counter_offset();

			UINT64 get_count()
			{
				return count;// get_desc().Width / sizeof(T);
			}
			using GPUBuffer::set_data;

			void set_data(DX12::CommandList::ptr& list, unsigned int offset, std::vector<type>& v);

			void set_data(DX12::CommandList::ptr& list, const type& v)
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


			void clear_counter(DX12::CommandList::ptr& list)
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
			void set_data(DX12::CommandList::ptr& list, unsigned int offset, const  std::vector<T>& v)
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
			void set_data(DX12::CommandList::ptr& list, unsigned int offset, const  std::vector<T>& v)
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


		class QueryHeap
		{

			ComPtr<ID3D12QueryHeap> heap;
		public:
			QueryHeap(UINT max_count, D3D12_QUERY_HEAP_TYPE type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP);

			ComPtr<ID3D12QueryHeap> get_native();

			using ptr = std::shared_ptr<QueryHeap>;
		};
		template<class T>
		inline void StructureBuffer<T>::init_views()
		{
			srv = StaticDescriptors::get().place(1);
			D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
			SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
			SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			SRVDesc.Buffer.NumElements = static_cast<UINT>(count);
			SRVDesc.Buffer.StructureByteStride = stride;
			SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

			Device::get().create_srv(srv[0], this, SRVDesc);


			hlsl = StaticDescriptors::get().place(4);

			structuredBuffer = HLSL::StructuredBuffer<T>(hlsl[0]);
			rwStructuredBuffer = HLSL::RWStructuredBuffer<T>(hlsl[1]);
			appendStructuredBuffer = HLSL::AppendStructuredBuffer<T>(hlsl[2]);

			structuredBufferCount = HLSL::StructuredBuffer<UINT>(hlsl[3]);
			Device::get().create_srv(structuredBuffer, this, SRVDesc);

			if (get_desc().Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
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
		inline StructureBuffer<T>::StructureBuffer(UINT64 count, counterType counted, D3D12_RESOURCE_FLAGS flags, HeapType heap_type, ResourceState defaultState) : GPUBuffer(counted == counterType::SELF ? (Math::AlignUp(count * sizeof(type), D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT) + (counted == counterType::SELF) * sizeof(UINT)) : (count * sizeof(type)), flags, defaultState, heap_type)
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
		inline void StructureBuffer<T>::set_data(DX12::CommandList::ptr& list, unsigned int offset, std::vector<type>& v)
		{
			list->get_copy().update_buffer(this, offset, reinterpret_cast<const char*>(v.data()), static_cast<UINT>(v.size() * sizeof(type)));
		}
		
		template<class T>
		inline void StructureBuffer<T>::place_uav(Handle  h)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
			desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.Buffer.NumElements = static_cast<UINT>(count);
			desc.Buffer.StructureByteStride = stride;
			desc.Buffer.CounterOffsetInBytes = 0;

			if (counted == counterType::SELF)
				desc.Buffer.CounterOffsetInBytes = std::max(0, (int)size - (int)sizeof(UINT));


			desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
			Resource* counter_resource = nullptr;

			if (counted == counterType::SELF) counter_resource = this;
			if (counted == counterType::HELP_BUFFER) counter_resource = help_buffer.get();


			Device::get().create_uav(h, this, desc, counter_resource);
		}
		template<class T>
		inline void StructureBuffer<T>::place_uav(Handle  h, GPUBuffer::ptr counter_resource, unsigned int offset)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
			desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.Buffer.NumElements = static_cast<UINT>(count);
			desc.Buffer.StructureByteStride = stride;
			desc.Buffer.CounterOffsetInBytes = offset;
			desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

			Device::get().create_uav(h, this, desc, counter_resource);
		}




	}

}