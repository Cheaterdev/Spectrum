#pragma once

namespace DX12
{


	class GPUBuffer: public Resource
	{
		protected:
			GPUBuffer() = default;

			GPUBuffer(UINT count, UINT stride);


			UINT count;
			UINT stride;

			UINT size;

			HandleTable uavs;
			HandleTable srv;
			HandleTable static_uav;
		
			void set_data(DX12::CommandList::ptr& list, unsigned int offset, const  std::string& v);


			void set_data(unsigned int offset, const std::string& v);
		public:
			virtual ~GPUBuffer()
			{
				size = 0;
			}
			using ptr = std::shared_ptr<GPUBuffer>;
			GPUBuffer(UINT size, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE);

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
				 srv = DescriptorHeapManager::get().get_csu_static()->create_table(1);
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


			void place_srv_buffer(const Handle& handle);

			void place_raw_uav(const Handle & h)
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
				desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
				desc.Format = DXGI_FORMAT_R32_TYPELESS;
				desc.Buffer.NumElements = get_desc().Width/4;
				desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
				Device::get().get_native_device()->CreateUnorderedAccessView(get_native().Get(), nullptr, &desc, h.cpu);
			}

			void place_structured_uav(const Handle& h, GPUBuffer::ptr counter_resource, unsigned int offset = 0);

			const HandleTable& get_srv();
			const HandleTable& get_uav();
			const HandleTable& get_static_uav();
			UINT get_count();

			template<class T>
			void set_raw_data(std::vector<T>& v)
			{
				auto list = Device::get().get_upload_list();
				list->transition(this, ResourceState::COPY_DEST);
				list->update_buffer(this, 0, reinterpret_cast<const char*>(v.data()), v.size() * sizeof(T));
				list->transition(this,  ResourceState::COMMON);
				list->end();
				list->execute();
			}

			template<class T>
			void set_data(const T& v)
			{
				auto list = Device::get().get_upload_list();
				list->transition(this,  ResourceState::COPY_DEST);
				list->update_buffer(this, 0, reinterpret_cast<const char*>(&v), sizeof(T));
				list->transition(this, ResourceState::COMMON);
				list->end();
				list->execute();
			}

			template<class T>
			void set_data(DX12::CommandList::ptr& list, std::vector<T>& v)
			{
				list->transition(this, ResourceState::COPY_DEST);
				list->update_buffer(this, 0, reinterpret_cast<const char*>(v.data()), v.size() * sizeof(T));
				//       list->transition(this, ResourceState::COPY_DEST, ResourceState::COMMON);
			}
			template<class T>
			void set_data(DX12::CommandList::ptr& list, const T&v)
			{
				list->transition(this, ResourceState::COPY_DEST);
				list->update_buffer(this, 0, reinterpret_cast<const char*>(&v), sizeof(T));
				//       list->transition(this, ResourceState::COPY_DEST, ResourceState::COMMON);
			}
			template<class T>
			void set_data(DX12::CommandList::ptr& list, UINT offset, const T* data, UINT size)
			{
				list->transition(this, ResourceState::COPY_DEST);
				list->update_buffer(this, offset, reinterpret_cast<const char*>(data), size * sizeof(T));
				//       list->transition(this, ResourceState::COPY_DEST, ResourceState::COMMON);
			}

		private:
			friend class boost::serialization::access;
			template<class Archive>
			void serialize(Archive& ar, const unsigned int)
			{
				if (Archive::is_loading::value)
				{
					ar& NVP(count);
					ar& NVP(stride);
					ar& NVP(size);
					std::string data;
					ar& NVP(data);
					init(CD3DX12_RESOURCE_DESC::Buffer(size), HeapType::DEFAULT, ResourceState::COMMON);
					set_data(0, data);
				}

				else
				{
					std::string data;
					auto list = Device::get().get_upload_list();
					list->transition(this, ResourceState::COPY_SOURCE);
					auto task =  list->read_buffer(this, 0, get_size(), [&data](const char* mem,  UINT64 size)
					{
						data.assign(mem, mem + size);
					});
					list->transition(this, ResourceState::COMMON);
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
			char* data;

			template<class T = char>
			T * map(int from, int to)
			{
				T* result = nullptr;
				D3D12_RANGE Range;
				Range.Begin = from ;
				Range.End = to;
				m_Resource->Map(0, &Range, reinterpret_cast<void**>(&result));
				return result;
			}


			void unmap();
		public:
			UploadBuffer(int count);
			virtual ~UploadBuffer();
			char* get_data();
	};


	class CPUBuffer: public Resource
	{
			int stride;
		public:
			CPUBuffer(int count, int stride);

			template<class T = char>
			T * map(int from, int to)
			{
				T* result = nullptr;
				D3D12_RANGE Range;
				Range.Begin = from * stride;
				Range.End = to * stride;
				m_Resource->Map(0, &Range, reinterpret_cast<void**>(&result));
				return result;
			}


			void unmap();

	};

	template<class T>
	class StructuredBuffer : public GPUBuffer
	{
			HandleTable static_raw_uav;

			StructuredBuffer() = default;
			bool counted = false;
			void init_views();
		public:
			using ptr = std::shared_ptr<StructuredBuffer<T>>;
			using type = T;
			StructuredBuffer(UINT count, bool counted = false, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE);

			UINT get_counter_offset();

			UINT get_count()
			{
				return count;// get_desc().Width / sizeof(T);
			}

			void set_data(DX12::CommandList::ptr& list, unsigned int offset, std::vector<T>& v);

			void set_data(DX12::CommandList::ptr& list, const T&v)
			{
				list->transition(this, ResourceState::COPY_DEST);
				list->update_buffer(this, 0, reinterpret_cast<const char*>(&v), sizeof(T));
				//       list->transition(this, ResourceState::COPY_DEST, ResourceState::COMMON);
			}

			void place_uav(const Handle& h);

			void place_uav(const Handle& h, GPUBuffer::ptr counter_resource, unsigned int offset = 0);

			const Handle& get_raw_uav() const
			{
				return static_raw_uav.get_base();
			}
		private:
			friend class boost::serialization::access;
			template<class Archive>
			void serialize(Archive& ar, const unsigned int)
			{
				ar& NVP(boost::serialization::base_object<GPUBuffer>(*this));
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
				list->transition(this, ResourceState::COPY_DEST);
				list->update_buffer(this, offset, reinterpret_cast<const char*>(v.data()), v.size() * sizeof(T));
				list->transition(this, ResourceState::COMMON);
			}

			template<class T>
			void set_data(unsigned int offset, const  std::vector<T>& v)
			{
				auto list = Device::get().get_upload_list();
				set_data(list, offset, v);
				list->end();
				list->execute_and_wait();
			}

			void place_uav(const Handle& h);
			void place_srv(const Handle& h);

			std::function<void(const Handle&)> uav()
			{
				return [this](const Handle & h)
				{
					place_uav(h);
				};
			}

		private:
			friend class boost::serialization::access;
			template<class Archive>
			void serialize(Archive& ar, const unsigned int)
			{
				ar& NVP(boost::serialization::base_object<GPUBuffer>(*this));
				init_views();
			}


	};

	class IndexBuffer : public GPUBuffer
	{
			IndexBuffer();;
		public:
			using ptr = std::shared_ptr<IndexBuffer>;

			explicit IndexBuffer(UINT size);
			template<class T>
			explicit IndexBuffer(const std::vector<T>& v) : GPUBuffer(v.size(), sizeof(T))
			{
				set_data(0, v);
			}
			template<class T>
			void set_data(DX12::CommandList::ptr& list, unsigned int offset, const  std::vector<T>& v)
			{
				list->transition(this,  ResourceState::COPY_DEST);
				list->update_buffer(this, offset, reinterpret_cast<const char*>(v.data()), v.size() * sizeof(T));
				list->transition(this,  ResourceState::COMMON);
			}

			template<class T>
			void set_data(unsigned int offset, const std::vector<T>& v)
			{
				auto list = Device::get().get_upload_list();
				set_data(list, offset, v);
				list->end();
				list->execute();
			}


			D3D12_INDEX_BUFFER_VIEW get_index_buffer_view(bool is_32, unsigned int offset = 0, unsigned int size = 0);

		private:
			friend class boost::serialization::access;
			template<class Archive>
			void serialize(Archive& ar, const unsigned int)
			{
				ar& NVP(boost::serialization::base_object<GPUBuffer>(*this));
			}


	};

	/*

	class BufferBase
	{
			//    Resource::ptr upload_resource;
		protected:
			char* data = nullptr;

			UINT size;
			Resource::ptr resource;
			bool changed = false;
			HeapType heaptype;
			virtual void update_internal(DX12::CommandList::ptr& list) {};

		public:
			using ptr = std::shared_ptr<BufferBase>;

			Resource::ptr get_resource()
			{
				return resource;
			}
			BufferBase(UINT size, HeapType heap, ResourceState state)
			{
				this->size = size;
				CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size);
				resource.reset(new Resource(desc, heap, state));
				heaptype = heap;

				if (heap == HeapType::UPLOAD)
					resource->get_native()->Map(0, nullptr, reinterpret_cast<void**>(&data));

				resource->set_name("BufferBase");
			}
			virtual ~BufferBase()
			{
				//     if (heaptype == HeapType::UPLOAD)
				//         resource->get_native()->Unmap(0, nullptr);
			}
			void update(DX12::CommandList::ptr& list)
			{
				update_internal(list);
			}


			virtual void update(DX12::CommandList* list)
			{
				update_internal(list->get_ptr());
			}

			UINT get_size()
			{
				return size;
			}

			char* get_data()
			{
				return data;
			}


			template<class T2>
			void set_data(T2* data, unsigned int count)
			{
				memcpy(this->data, data, count * sizeof(T2));
				changed = true;
			}

			template<class T2>
			void set_data(DX12::CommandList::ptr& list, unsigned int offset, const T2* data, unsigned int count)
			{
				list->transition(resource, ResourceState::GEN_READ, ResourceState::COPY_DEST);
				list->update_buffer(resource, offset, reinterpret_cast<const char*>(data), count * sizeof(T2));
				list->transition(resource, ResourceState::COPY_DEST, ResourceState::GEN_READ);
			}

			template<class T2>
			void set_data(DX12::CommandList::ptr& list, unsigned int offset, std::vector<T2>& v)
			{
				list->transition(resource, ResourceState::GEN_READ, ResourceState::COPY_DEST);
				list->update_buffer(resource, offset, reinterpret_cast<const char*>(v.data()), v.size() * sizeof(T2));
				list->transition(resource, ResourceState::COPY_DEST, ResourceState::GEN_READ);
			}

			virtual   D3D12_VERTEX_BUFFER_VIEW get_vertex_buffer_view()
			{
				assert(false);
				// unused
				D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
				m_vertexBufferView.BufferLocation = resource->get_gpu_address();
				m_vertexBufferView.SizeInBytes = size;
				m_vertexBufferView.StrideInBytes = 0;
				return m_vertexBufferView;
			}


			virtual   D3D12_INDEX_BUFFER_VIEW get_index_buffer_view()
			{
				assert(false);
				// unused
				D3D12_INDEX_BUFFER_VIEW m_vertexBufferView;
				m_vertexBufferView.BufferLocation = resource->get_gpu_address();
				m_vertexBufferView.SizeInBytes = size;
				m_vertexBufferView.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
				return m_vertexBufferView;
			}
	};
	*/
	/*
	template<class T>
	class Buffer : public BufferBase
	{
			std::vector<char> cache;
			UINT count;
			T* typed_data = nullptr;
			UINT stride;

		public:
			using ptr = std::shared_ptr<Buffer<T>>;
			Buffer(HeapType type = HeapType::DEFAULT, UINT count = 1, UINT stride = sizeof(T)) : BufferBase(count * sizeof(T), type, DX12::ResourceState::GEN_READ)
			{
				this->count = count;
				this->stride = stride;

				if (!data)
				{
					cache.resize(size);
					data = cache.data();
				}

				typed_data = reinterpret_cast<T*>(data);
				resource->set_name("Buffer<T>");
			}


			T* operator->()
			{
				changed = true;
				return reinterpret_cast<T*>(data);
			}

			template <class T2>
			const T& operator=(const T2& r)
			{
				changed = true;
				return (*reinterpret_cast<T*>(data)) = r;
			}


			T& operator[](unsigned int i)
			{
				changed = true;
				T& obj = typed_data[i];
				return obj;
			}



			void place_const_buffer(const Handle& handle)
			{
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
				cbvDesc.SizeInBytes = (size + (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
				cbvDesc.BufferLocation = resource->get_native()->GetGPUVirtualAddress();
				Device::get().get_native_device()->CreateConstantBufferView(&cbvDesc, handle.cpu);
			}
			void place_structured_srv(const Handle& handle)
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
				SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
				SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
				SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				SRVDesc.Buffer.NumElements = count;
				SRVDesc.Buffer.StructureByteStride = stride;
				SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
				Device::get().get_native_device()->CreateShaderResourceView(resource->get_native().Get(), &SRVDesc, handle.cpu);
			}

			void place_srv_buffer(const Handle& handle)
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC  srv = {};
				srv.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
				srv.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				srv.Buffer.FirstElement = 0;
				srv.Buffer.Flags = D3D12_BUFFER_SRV_FLAGS::D3D12_BUFFER_SRV_FLAG_NONE;
				srv.Buffer.StructureByteStride = 0;
				srv.Buffer.NumElements = count;
				Device::get().get_native_device()->CreateShaderResourceView(resource->get_native().Get(), &srv, handle.cpu);
			}

			//virtual void update_data(DX12::CommandList::ptr list)

			virtual void update_internal(DX12::CommandList::ptr& list) override
			{
				if (!changed) return;

				if (cache.size())
				{
					list->transition(resource, ResourceState::GEN_READ, ResourceState::COPY_DEST);
					list->update_buffer(resource, 0, (data), size);
					list->transition(resource, ResourceState::COPY_DEST, ResourceState::GEN_READ);
					changed = false;
				}
			}

			D3D12_VERTEX_BUFFER_VIEW get_vertex_buffer_view() override
			{
				D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
				m_vertexBufferView.BufferLocation = resource->get_gpu_address();
				m_vertexBufferView.SizeInBytes = size;
				m_vertexBufferView.StrideInBytes = stride;
				return m_vertexBufferView;
			}




	};
	*/

	/*
	class VertexBufferBase : public BufferBase
	{
		protected:
			D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
		public:
			using ptr = std::shared_ptr<VertexBufferBase>;
			VertexBufferBase(int size) : BufferBase(size, DX12::HeapType::DEFAULT, DX12::ResourceState::VERTEX_AND_CONSTANT_BUFFER)
			{
			}

			D3D12_VERTEX_BUFFER_VIEW get_view()
			{
				return m_vertexBufferView;
			}

	};
	template<class T>
	class VertexBuffer : public VertexBufferBase
	{
			std::vector<T> data;

			friend class CommandList;

		public:
			using ptr = std::shared_ptr<VertexBuffer<T>>;
			VertexBuffer(UINT count = 1) : VertexBufferBase(count * sizeof(T))
			{
				data.resize(count);
				m_vertexBufferView.BufferLocation = resource->get_gpu_address();
				m_vertexBufferView.SizeInBytes = size;
				m_vertexBufferView.StrideInBytes = sizeof(T);
			}

			T* operator->()
			{
				changed = true;
				return data.data();
			}

			template<class T2>
			void set_data(T2* data, unsigned int count)
			{
				memcpy(this->data.data(), data, count * sizeof(T2));
				changed = true;
			}




			T& operator[](unsigned int i)
			{
				changed = true;
				return data[i];
			}

			virtual void update_internal(DX12::CommandList::ptr& list)
			{
				if (!changed) return;

				D3D12_SUBRESOURCE_DATA vertexData = {};
				vertexData.pData = data.data();
				vertexData.RowPitch = size;
				vertexData.SlicePitch = size;
				list->transition(resource, ResourceState::GEN_READ, ResourceState::COPY_DEST);
				list->update_resource(resource, 0, 1, &vertexData);
				list->transition(resource, ResourceState::COPY_DEST, ResourceState::GEN_READ);
				changed = false;
			}

	};
	*/

	class QueryHeap
	{

			CComPtr<ID3D12QueryHeap> heap;
		public:
			QueryHeap(UINT max_count, D3D12_QUERY_HEAP_TYPE type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP);

			CComPtr<ID3D12QueryHeap> get_native();

			using ptr = std::shared_ptr<QueryHeap>;
	};
	template<class T>
	inline void StructuredBuffer<T>::init_views()
	{
		srv = DescriptorHeapManager::get().get_csu_static()->create_table(1);
		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.Buffer.NumElements = count;
		SRVDesc.Buffer.StructureByteStride = stride;
		SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		Device::get().get_native_device()->CreateShaderResourceView(get_native().Get(), &SRVDesc, srv[0].cpu);

		if (get_desc().Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
		{
			uavs = DescriptorHeapManager::get().get_csu_static()->create_table(1);
			place_uav(uavs[0]);
			static_uav = DescriptorHeapManager::get().get_csu_static()->create_table(1);
			place_uav(static_uav[0]);
			static_raw_uav = DescriptorHeapManager::get().get_csu_static()->create_table(1);

			place_raw_uav(static_raw_uav[0]);

		}
	}
	template<class T>
	inline StructuredBuffer<T>::StructuredBuffer(UINT count, bool counted, D3D12_RESOURCE_FLAGS flags) : GPUBuffer(counted ? (Math::AlignUp(count * sizeof(T), D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT) + counted * sizeof(UINT)) : (count * sizeof(T)), flags)
	{
		stride = sizeof(T);
		this->count = count;
		this->counted = counted;
		init_views();
	}
	template<class T>
	inline UINT StructuredBuffer<T>::get_counter_offset()
	{
		assert(counted, "needs to be counted");
		return size - sizeof(UINT);
	}
	template<class T>
	inline void StructuredBuffer<T>::set_data(DX12::CommandList::ptr & list, unsigned int offset, std::vector<T>& v)
	{
		list->transition(this,ResourceState::COPY_DEST);
		list->update_buffer(this, offset, reinterpret_cast<const char*>(v.data()), v.size() * sizeof(T));
		list->transition(this,  ResourceState::COMMON);
	}
	template<class T>
	inline void StructuredBuffer<T>::place_uav(const Handle & h)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
		desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Buffer.NumElements = count;
		desc.Buffer.StructureByteStride = stride;
		desc.Buffer.CounterOffsetInBytes = counted ? size - sizeof(UINT) : 0;
		desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		Device::get().get_native_device()->CreateUnorderedAccessView(get_native().Get(), counted ? get_native().Get() : nullptr, &desc, h.cpu);
	}
	template<class T>
	inline void StructuredBuffer<T>::place_uav(const Handle & h, GPUBuffer::ptr counter_resource, unsigned int offset)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
		desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Buffer.NumElements = count;
		desc.Buffer.StructureByteStride = stride;
		desc.Buffer.CounterOffsetInBytes = offset;
		desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		Device::get().get_native_device()->CreateUnorderedAccessView(get_native().Get(), counter_resource ? counter_resource->get_native().Get() : nullptr, &desc, h.cpu);
	}
}