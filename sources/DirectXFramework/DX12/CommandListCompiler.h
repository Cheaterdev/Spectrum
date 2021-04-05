#pragma once

namespace DX12
{

	struct CommandListCompiled
	{
		ComPtr<ID3D12GraphicsCommandList4> m_commandList;
		ComPtr<ID3D12CommandAllocator> m_commandAllocator;

		operator bool()
		{
			return !!m_commandList;
		}
	};



	class CommandListCompiler
	{
		CommandListCompiled compiled;


	public:
		CommandListCompiler()
		{

		}

		void create(CommandListType type);

		void reset();
		CommandListCompiled compile();

		virtual HRESULT  SetName(
			LPCWSTR Name)
		{
			return 0;
		};

		virtual void  BuildRaytracingAccelerationStructure(
			const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC* pDesc,
			UINT NumPostbuildInfoDescs,
			const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC* pPostbuildInfoDescs)
		{
			compiled.m_commandList->BuildRaytracingAccelerationStructure(pDesc, NumPostbuildInfoDescs, pPostbuildInfoDescs);
		};


		virtual void  SetPipelineState1(
			ID3D12StateObject* pStateObject)
		{
			compiled.m_commandList->SetPipelineState1(pStateObject);
		};

		virtual void  DispatchRays(
			const D3D12_DISPATCH_RAYS_DESC* pDesc) {
			compiled.m_commandList->DispatchRays(pDesc);
		};

		void  ClearState(
			ID3D12PipelineState* pPipelineState)
		{
			compiled.m_commandList->ClearState(pPipelineState);
		}

		void  DrawInstanced(
			UINT VertexCountPerInstance,
			UINT InstanceCount,
			UINT StartVertexLocation,
			UINT StartInstanceLocation)
		{
			compiled.m_commandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
		}

		void  DrawIndexedInstanced(
			UINT IndexCountPerInstance,
			UINT InstanceCount,
			UINT StartIndexLocation,
			INT BaseVertexLocation,
			UINT StartInstanceLocation)
		{
			compiled.m_commandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
		}

		void  Dispatch(
			UINT ThreadGroupCountX,
			UINT ThreadGroupCountY,
			UINT ThreadGroupCountZ)
		{
			compiled.m_commandList->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
		}

		void  CopyBufferRegion(
			ID3D12Resource* pDstBuffer,
			UINT64 DstOffset,
			ID3D12Resource* pSrcBuffer,
			UINT64 SrcOffset,
			UINT64 NumBytes)
		{
			compiled.m_commandList->CopyBufferRegion(pDstBuffer, DstOffset, pSrcBuffer, SrcOffset, NumBytes);
		}

		void  CopyTextureRegion(
			const D3D12_TEXTURE_COPY_LOCATION* pDst,
			UINT DstX,
			UINT DstY,
			UINT DstZ,
			const D3D12_TEXTURE_COPY_LOCATION* pSrc,
			const D3D12_BOX* pSrcBox)
		{
			compiled.m_commandList->CopyTextureRegion(pDst, DstX, DstY, DstZ, pSrc, pSrcBox);
		}

		void  CopyResource(
			ID3D12Resource* pDstResource,
			ID3D12Resource* pSrcResource)
		{
			compiled.m_commandList->CopyResource(pDstResource, pSrcResource);
		}

		void  IASetPrimitiveTopology(
			D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology)
		{
			compiled.m_commandList->IASetPrimitiveTopology(PrimitiveTopology);
		}

		void  RSSetViewports(
			UINT NumViewports,
			const D3D12_VIEWPORT* pViewports) {
			compiled.m_commandList->RSSetViewports(NumViewports, pViewports);
		}

		void  RSSetScissorRects(
			UINT NumRects,
			const D3D12_RECT* pRects)
		{
			compiled.m_commandList->RSSetScissorRects(NumRects, pRects);
		}

		void  OMSetBlendFactor(
			const FLOAT BlendFactor[4]) {
			compiled.m_commandList->OMSetBlendFactor(BlendFactor);
		}

		void  OMSetStencilRef(
			UINT StencilRef) {
			compiled.m_commandList->OMSetStencilRef(StencilRef);
		}

		void  SetPipelineState(
			ID3D12PipelineState* pPipelineState) {
			compiled.m_commandList->SetPipelineState(pPipelineState);
		}

		void  ResourceBarrier(
			UINT NumBarriers,
			const D3D12_RESOURCE_BARRIER* pBarriers)
		{
			compiled.m_commandList->ResourceBarrier(NumBarriers, pBarriers);
		}

		void  ExecuteBundle(
			ID3D12GraphicsCommandList* pCommandList)
		{
			compiled.m_commandList->ExecuteBundle(pCommandList);
		}

		void  SetDescriptorHeaps(
			UINT NumDescriptorHeaps,
			ID3D12DescriptorHeap* const* ppDescriptorHeaps)
		{
			compiled.m_commandList->SetDescriptorHeaps(NumDescriptorHeaps, ppDescriptorHeaps);
		}

		void  SetComputeRootSignature(
			ID3D12RootSignature* pRootSignature)
		{
			compiled.m_commandList->SetComputeRootSignature(pRootSignature);
		}

		void  SetGraphicsRootSignature(
			ID3D12RootSignature* pRootSignature)
		{
			compiled.m_commandList->SetGraphicsRootSignature(pRootSignature);
		}

		void  SetComputeRootDescriptorTable(
			UINT RootParameterIndex,
			D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
		{
			compiled.m_commandList->SetComputeRootDescriptorTable(RootParameterIndex, BaseDescriptor);
		}

		void  SetGraphicsRootDescriptorTable(
			UINT RootParameterIndex,
			D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
		{
			compiled.m_commandList->SetGraphicsRootDescriptorTable(RootParameterIndex, BaseDescriptor);
		}

		virtual void  SetComputeRootConstantBufferView(
			UINT RootParameterIndex,
			D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
		{
			compiled.m_commandList->SetComputeRootConstantBufferView(RootParameterIndex, BufferLocation);
		}


		virtual void  SetGraphicsRootConstantBufferView(
			UINT RootParameterIndex,
			D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
		{
			compiled.m_commandList->SetGraphicsRootConstantBufferView(RootParameterIndex, BufferLocation);
		}


		void  IASetIndexBuffer(
			const D3D12_INDEX_BUFFER_VIEW* pView)
		{
			compiled.m_commandList->IASetIndexBuffer(pView);
		}

		void  IASetVertexBuffers(
			UINT StartSlot,
			UINT NumViews,
			const D3D12_VERTEX_BUFFER_VIEW* pViews)
		{
			compiled.m_commandList->IASetVertexBuffers(StartSlot, NumViews, pViews);
		}

		void  SOSetTargets(
			UINT StartSlot,
			UINT NumViews,
			const D3D12_STREAM_OUTPUT_BUFFER_VIEW* pViews)
		{
			compiled.m_commandList->SOSetTargets(StartSlot, NumViews, pViews);
		}

		void  OMSetRenderTargets(
			UINT NumRenderTargetDescriptors,
			const D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors,
			BOOL RTsSingleHandleToDescriptorRange,
			const D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor)
		{
			compiled.m_commandList->OMSetRenderTargets(NumRenderTargetDescriptors, pRenderTargetDescriptors, RTsSingleHandleToDescriptorRange, pDepthStencilDescriptor);
		}

		void  ClearDepthStencilView(
			D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView,
			D3D12_CLEAR_FLAGS ClearFlags,
			FLOAT Depth,
			UINT8 Stencil,
			UINT NumRects,
			const D3D12_RECT* pRects)
		{
			compiled.m_commandList->ClearDepthStencilView(DepthStencilView, ClearFlags, Depth, Stencil, NumRects, pRects);
		}

		void  ClearRenderTargetView(
			D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView,
			const FLOAT ColorRGBA[4],
			UINT NumRects,
			const D3D12_RECT* pRects)
		{
			compiled.m_commandList->ClearRenderTargetView(RenderTargetView, ColorRGBA, NumRects, pRects);
		}

		void  ClearUnorderedAccessViewUint(
			D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap,
			D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle,
			ID3D12Resource* pResource,
			const UINT Values[4],
			UINT NumRects,
			const D3D12_RECT* pRects)
		{
			compiled.m_commandList->ClearUnorderedAccessViewUint(ViewGPUHandleInCurrentHeap, ViewCPUHandle, pResource, Values, NumRects, pRects);
		}

		void  ClearUnorderedAccessViewFloat(
			D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap,
			D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle,
			ID3D12Resource* pResource,
			const FLOAT Values[4],
			UINT NumRects,
			const D3D12_RECT* pRects)
		{
			compiled.m_commandList->ClearUnorderedAccessViewFloat(ViewGPUHandleInCurrentHeap, ViewCPUHandle, pResource, Values, NumRects, pRects);
		}

		void  DiscardResource(
			ID3D12Resource* pResource,
			const D3D12_DISCARD_REGION* pRegion)
		{
			compiled.m_commandList->DiscardResource(pResource, pRegion);
		}

		void  BeginQuery(
			ID3D12QueryHeap* pQueryHeap,
			D3D12_QUERY_TYPE Type,
			UINT Index)
		{
			compiled.m_commandList->BeginQuery(pQueryHeap, Type, Index);
		}

		void  EndQuery(
			ID3D12QueryHeap* pQueryHeap,
			D3D12_QUERY_TYPE Type,
			UINT Index)
		{
			compiled.m_commandList->EndQuery(pQueryHeap, Type, Index);
		}

		void  ResolveQueryData(
			ID3D12QueryHeap* pQueryHeap,
			D3D12_QUERY_TYPE Type,
			UINT StartIndex,
			UINT NumQueries,
			ID3D12Resource* pDestinationBuffer,
			UINT64 AlignedDestinationBufferOffset)
		{
			compiled.m_commandList->ResolveQueryData(pQueryHeap, Type, StartIndex, NumQueries, pDestinationBuffer, AlignedDestinationBufferOffset);
		}

		void  SetPredication(
			ID3D12Resource* pBuffer,
			UINT64 AlignedBufferOffset,
			D3D12_PREDICATION_OP Operation)
		{
			compiled.m_commandList->SetPredication(pBuffer, AlignedBufferOffset, Operation);
		}

		void  SetMarker(
			UINT Metadata,
			const void* pData,
			UINT Size)
		{
			compiled.m_commandList->SetMarker(Metadata, pData, Size);
		}

		void  BeginEvent(
			UINT Metadata,
			const void* pData,
			UINT Size)
		{
			compiled.m_commandList->BeginEvent(Metadata, pData, Size);
		}

		void  EndEvent(void)
		{
			compiled.m_commandList->EndEvent();
		}

		void  ExecuteIndirect(
			ID3D12CommandSignature* pCommandSignature,
			UINT MaxCommandCount,
			ID3D12Resource* pArgumentBuffer,
			UINT64 ArgumentBufferOffset,
			ID3D12Resource* pCountBuffer,
			UINT64 CountBufferOffset)
		{
			compiled.m_commandList->ExecuteIndirect(pCommandSignature, MaxCommandCount, pArgumentBuffer, ArgumentBufferOffset, pCountBuffer, CountBufferOffset);
		}

	};



	class CommandsAllocator
	{

		std::vector<std::byte> data;
	protected:
		uint write_offset = 0;
		uint read_offset = 0;
	public:
		CommandsAllocator()
		{
			data.resize(1024 * 1024 * 4);
		}

		void request(UINT size)
		{
			assert(data.size() > (write_offset + size));

		}

		template <class T>
		void push_one(const T& elem)
		{
			request(sizeof(T));
			memcpy(data.data() + write_offset, &elem, sizeof(T));
			write_offset += sizeof(T);
			
		}

		template <class ...Args>
		void push(Args... args)
		{
			(push_one(std::forward<Args>(args)),...);
		}

		template <class T>
		void push_array(const T* v, UINT n)
		{
			push(v != nullptr);

			if (v)
				for (int i = 0; i < n; i++)
				{

					push(v[i]);

				}
		}

		template <class T, UINT N>
		void push_array(const T(v)[N])
		{
			push(v != nullptr);

			if (v)
				for (int i = 0; i < N; i++)
				{

					push(v[i]);

				}
		}

		template <class T>
		void push_opt(const T* v)
		{
			push(v != nullptr);

			if (v)	push(*v);
		}


	

		template <class T>
		void pop_opt(T*& v)
		{
			bool b;
			pop(b);
			if (b)
			{
				auto ptr = reinterpret_cast<T*>(data.data() + read_offset);
				read_offset += sizeof(T);
				v = ptr;
			}else
			v = nullptr;

		}


		template <class T>
		void pop_array(T*& v, UINT n)
		{
			bool b;
			pop(b);
			if (b)
			{
				auto ptr = reinterpret_cast<T*>(data.data() + read_offset);
				read_offset += sizeof(T) * n;

				v = ptr;
			}
			else

			v = nullptr;
		}



		

		template <class T, UINT N>
		void pop_array(T(&v)[N])
		{
			bool b;
			pop(b);
			if (b)
			{
				auto ptr = reinterpret_cast<T*>(data.data() + read_offset);
				read_offset += sizeof(T) * N;

				v = ptr;
			}
			else
			v = nullptr;
		}

		

		template<class T>
		void pop_one(T& v)
		{
			auto ptr = reinterpret_cast<T*>(data.data() + read_offset);
			read_offset += sizeof(T);

			v = *ptr;
		}

		template <class ...Args>
		void pop(Args&... args)
		{
			(pop_one(args), ...);
		}
		
		template<class T>
		void pop_ptr(T*& v)
		{
			auto ptr = reinterpret_cast<T*>(data.data() + read_offset);
			read_offset += sizeof(T);

			v = ptr;
		}
	};

	class CommandListCompilerDelayed :protected CommandsAllocator
	{
		CommandListCompiled compiled;

		std::list<std::function<void()>> tasks;
	public:
		CommandListCompilerDelayed()
		{

		}

		void create(CommandListType type);

		void reset();
		CommandListCompiled compile();

		virtual HRESULT  SetName(
			LPCWSTR Name)
		{
			return 0;
		};


		void func(std::function<void(ID3D12GraphicsCommandList4* list)>  f)
		{
			tasks.emplace_back([this, f]()
			{
					f(compiled.m_commandList.Get());
			});
		}
		virtual void  BuildRaytracingAccelerationStructure(
			const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC* pDesc,
			UINT NumPostbuildInfoDescs,
			const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC* pPostbuildInfoDescs)
		{
			push(*pDesc);
			push(NumPostbuildInfoDescs);
			push_array(pPostbuildInfoDescs, NumPostbuildInfoDescs);

			if (pDesc->Inputs.Type == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE::D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL) {
				if (pDesc->Inputs.DescsLayout == D3D12_ELEMENTS_LAYOUT::D3D12_ELEMENTS_LAYOUT_ARRAY)
				{
					push_array(pDesc->Inputs.pGeometryDescs, pDesc->Inputs.NumDescs);
				}
				else 	if (pDesc->Inputs.DescsLayout == D3D12_ELEMENTS_LAYOUT::D3D12_ELEMENTS_LAYOUT_ARRAY_OF_POINTERS)
				{
					for (int i = 0; i < pDesc->Inputs.NumDescs; i++)
					{
						push(*pDesc->Inputs.ppGeometryDescs[i]);
					}
				}
			}
		//	
			
			tasks.emplace_back([this]()
				{
					D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC* pDesc;
					UINT NumPostbuildInfoDescs;
					D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC* pPostbuildInfoDescs;

					pop_ptr(pDesc);
					pop(NumPostbuildInfoDescs);
					pop_array(pPostbuildInfoDescs, NumPostbuildInfoDescs);

				
					if (pDesc->Inputs.Type == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE::D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL) {

						if (pDesc->Inputs.DescsLayout == D3D12_ELEMENTS_LAYOUT::D3D12_ELEMENTS_LAYOUT_ARRAY)
						{
							pop_array(pDesc->Inputs.pGeometryDescs, pDesc->Inputs.NumDescs);
						}
						else if (pDesc->Inputs.DescsLayout == D3D12_ELEMENTS_LAYOUT::D3D12_ELEMENTS_LAYOUT_ARRAY_OF_POINTERS)
						{
							pDesc->Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT::D3D12_ELEMENTS_LAYOUT_ARRAY;

							pop_array(pDesc->Inputs.pGeometryDescs, pDesc->Inputs.NumDescs);
						}
					}
				
					compiled.m_commandList->BuildRaytracingAccelerationStructure(pDesc, NumPostbuildInfoDescs, pPostbuildInfoDescs);
				});

		};


		virtual void  SetPipelineState1(
			ID3D12StateObject* pStateObject)
		{
			push(pStateObject);
			tasks.emplace_back([this]()
				{
					ID3D12StateObject* pStateObject;
					pop(pStateObject);
					compiled.m_commandList->SetPipelineState1(pStateObject);
				});
		};

		virtual void  DispatchRays(
			const D3D12_DISPATCH_RAYS_DESC* pDesc) {

			push(*pDesc);
			tasks.emplace_back([this]()
				{
					D3D12_DISPATCH_RAYS_DESC* pDesc;

					pop_ptr(pDesc);
					compiled.m_commandList->DispatchRays(pDesc);
				});
		};

		void  ClearState(
			ID3D12PipelineState* pPipelineState)
		{
			push(pPipelineState);
			tasks.emplace_back([this]()
				{
					ID3D12PipelineState* pPipelineState;
					pop(pPipelineState);
					compiled.m_commandList->ClearState(pPipelineState);
				});

		}

		void  DrawInstanced(
			UINT VertexCountPerInstance,
			UINT InstanceCount,
			UINT StartVertexLocation,
			UINT StartInstanceLocation)
		{
			push(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
			tasks.emplace_back([this]()
				{

					UINT VertexCountPerInstance;
					UINT InstanceCount;
					UINT StartVertexLocation;
					UINT StartInstanceLocation;
					pop(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);


					compiled.m_commandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
				});
		}

		void  DrawIndexedInstanced(
			UINT IndexCountPerInstance,
			UINT InstanceCount,
			UINT StartIndexLocation,
			INT BaseVertexLocation,
			UINT StartInstanceLocation)
		{

			push(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);


			tasks.emplace_back([this]()
				{
					UINT IndexCountPerInstance;
					UINT InstanceCount;
					UINT StartIndexLocation;
					INT BaseVertexLocation;
					UINT StartInstanceLocation;

					pop(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);

					compiled.m_commandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
				});
		}

		void  Dispatch(
			UINT ThreadGroupCountX,
			UINT ThreadGroupCountY,
			UINT ThreadGroupCountZ)
		{
			push(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);

			tasks.emplace_back([this]()
				{
					UINT ThreadGroupCountX;
					UINT ThreadGroupCountY;
					UINT ThreadGroupCountZ;
					pop(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);


					compiled.m_commandList->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
				});

		}

		void  CopyBufferRegion(
			ID3D12Resource* pDstBuffer,
			UINT64 DstOffset,
			ID3D12Resource* pSrcBuffer,
			UINT64 SrcOffset,
			UINT64 NumBytes)
		{
			push(pDstBuffer, DstOffset, pSrcBuffer, SrcOffset, NumBytes);


			tasks.emplace_back([this]()
				{
					ID3D12Resource* pDstBuffer;
					UINT64 DstOffset;
					ID3D12Resource* pSrcBuffer;
					UINT64 SrcOffset;
					UINT64 NumBytes;
					pop(pDstBuffer, DstOffset, pSrcBuffer, SrcOffset, NumBytes);

					compiled.m_commandList->CopyBufferRegion(pDstBuffer, DstOffset, pSrcBuffer, SrcOffset, NumBytes);
				});

		}

		void  CopyTextureRegion(
			const D3D12_TEXTURE_COPY_LOCATION* pDst,
			UINT DstX,
			UINT DstY,
			UINT DstZ,
			const D3D12_TEXTURE_COPY_LOCATION* pSrc,
			const D3D12_BOX* pSrcBox)
		{
			push(*pDst, *pSrc);

			push(DstX, DstY, DstZ);
			push_opt(pSrcBox);

			tasks.emplace_back([this]()
				{
					 D3D12_TEXTURE_COPY_LOCATION* pDst;
					UINT DstX;
					UINT DstY;
					UINT DstZ;
					 D3D12_TEXTURE_COPY_LOCATION* pSrc;
					 D3D12_BOX* pSrcBox;
					 pop_ptr(pDst);
					 pop_ptr(pSrc);
					pop( DstX, DstY, DstZ);
					pop_opt(pSrcBox);
					compiled.m_commandList->CopyTextureRegion(pDst, DstX, DstY, DstZ, pSrc, pSrcBox);
				});


		}

		void  CopyResource(
			ID3D12Resource* pDstResource,
			ID3D12Resource* pSrcResource)
		{
			push(pDstResource, pSrcResource);
			tasks.emplace_back([this]()
				{
					ID3D12Resource* pDstResource;
					ID3D12Resource* pSrcResource;
					pop(pDstResource, pSrcResource);
					compiled.m_commandList->CopyResource(pDstResource, pSrcResource);
				});
		}

		void  IASetPrimitiveTopology(
			D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology)
		{
			push(PrimitiveTopology);

			tasks.emplace_back([this]()
				{
					D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology;
					pop(PrimitiveTopology);
					compiled.m_commandList->IASetPrimitiveTopology(PrimitiveTopology);
				});
		}

		void  RSSetViewports(
			UINT NumViewports,
			const D3D12_VIEWPORT* pViewports) {

			push(NumViewports);
			push_array(pViewports, NumViewports);


			tasks.emplace_back([this]()
				{
					UINT NumViewports;
					 D3D12_VIEWPORT* pViewports;
					pop(NumViewports);
					pop_array(pViewports, NumViewports);

					compiled.m_commandList->RSSetViewports(NumViewports, pViewports);
				});

		}

		void  RSSetScissorRects(
			UINT NumRects,
			const D3D12_RECT* pRects)
		{
			push(NumRects);
			push_array(pRects, NumRects);

			tasks.emplace_back([this]()
				{
					UINT NumRects;
					 D3D12_RECT* pRects;
					pop(NumRects);
					pop_array(pRects, NumRects);
					compiled.m_commandList->RSSetScissorRects(NumRects, pRects);
				});
		}

		void  OMSetBlendFactor(
			const FLOAT BlendFactor[4]) {

			push_array(BlendFactor,4);


			tasks.emplace_back([this]()
				{
					FLOAT* BlendFactor;
					pop_array(BlendFactor,4);

					compiled.m_commandList->OMSetBlendFactor(BlendFactor);
				});
		}

		void  OMSetStencilRef(
			UINT StencilRef) {

			push(StencilRef);

			tasks.emplace_back([this]()
				{
					UINT StencilRef;
					pop(StencilRef);

					compiled.m_commandList->OMSetStencilRef(StencilRef);
				});
		}

		void  SetPipelineState(
			ID3D12PipelineState* pPipelineState) {
			push(pPipelineState);

			tasks.emplace_back([this]()
				{	ID3D12PipelineState* pPipelineState;
			pop(pPipelineState);
			compiled.m_commandList->SetPipelineState(pPipelineState);
				});
		}

		void  ResourceBarrier(
			UINT NumBarriers,
			const D3D12_RESOURCE_BARRIER* pBarriers)
		{
			push(NumBarriers);
			push_array(pBarriers, NumBarriers);

			tasks.emplace_back([this]()
				{
					UINT NumBarriers;
						const D3D12_RESOURCE_BARRIER* pBarriers;
						pop(NumBarriers);
						pop_array(pBarriers, NumBarriers);

				
					compiled.m_commandList->ResourceBarrier(NumBarriers, pBarriers);
				});
		}

		void  ExecuteBundle(
			ID3D12GraphicsCommandList* pCommandList)
		{
			push(pCommandList);

			tasks.emplace_back([this]()
				{
					ID3D12GraphicsCommandList* pCommandList;
					pop(pCommandList);
				
					compiled.m_commandList->ExecuteBundle(pCommandList);
				});
		}

		void  SetDescriptorHeaps(
			UINT NumDescriptorHeaps,
			ID3D12DescriptorHeap* const* ppDescriptorHeaps)
		{

			push(NumDescriptorHeaps);
			push_array(ppDescriptorHeaps, NumDescriptorHeaps);

			tasks.emplace_back([this]()
				{
					UINT NumDescriptorHeaps;
						ID3D12DescriptorHeap* const* ppDescriptorHeaps;

						pop(NumDescriptorHeaps);
						pop_array(ppDescriptorHeaps, NumDescriptorHeaps);

					compiled.m_commandList->SetDescriptorHeaps(NumDescriptorHeaps, ppDescriptorHeaps);
				});
		}

		void  SetComputeRootSignature(
			ID3D12RootSignature* pRootSignature)
		{
			push(pRootSignature);

			tasks.emplace_back([this]()
				{
					ID3D12RootSignature* pRootSignature;
					pop(pRootSignature);

					compiled.m_commandList->SetComputeRootSignature(pRootSignature);
				});
		}

		void  SetGraphicsRootSignature(
			ID3D12RootSignature* pRootSignature)
		{
			push(pRootSignature);

			tasks.emplace_back([this]()
				{	
				ID3D12RootSignature* pRootSignature;
			pop(pRootSignature);

					compiled.m_commandList->SetGraphicsRootSignature(pRootSignature);
				});
		}

		void  SetComputeRootDescriptorTable(
			UINT RootParameterIndex,
			D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
		{
			push(RootParameterIndex, BaseDescriptor);


			tasks.emplace_back([this]()
				{
					UINT RootParameterIndex;
					D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor;
					pop(RootParameterIndex, BaseDescriptor);

					compiled.m_commandList->SetComputeRootDescriptorTable(RootParameterIndex, BaseDescriptor);
				});
		}

		void  SetGraphicsRootDescriptorTable(
			UINT RootParameterIndex,
			D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
		{
			push(RootParameterIndex, BaseDescriptor);

			tasks.emplace_back([this]()
				{
					UINT RootParameterIndex;
					D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor;
					pop(RootParameterIndex, BaseDescriptor);
				
					compiled.m_commandList->SetGraphicsRootDescriptorTable(RootParameterIndex, BaseDescriptor);
				});
		}

		virtual void  SetComputeRootConstantBufferView(
			UINT RootParameterIndex,
			D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
		{
			push(RootParameterIndex, BufferLocation);

			tasks.emplace_back([this]()
				{
					UINT RootParameterIndex;
					D3D12_GPU_VIRTUAL_ADDRESS BufferLocation;
					pop(RootParameterIndex, BufferLocation);

				
					compiled.m_commandList->SetComputeRootConstantBufferView(RootParameterIndex, BufferLocation);
				});
		}


		virtual void  SetGraphicsRootConstantBufferView(
			UINT RootParameterIndex,
			D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
		{
			push(RootParameterIndex, BufferLocation);

			tasks.emplace_back([this]()
				{
					UINT RootParameterIndex;
					D3D12_GPU_VIRTUAL_ADDRESS BufferLocation;
					pop(RootParameterIndex, BufferLocation);
				
					compiled.m_commandList->SetGraphicsRootConstantBufferView(RootParameterIndex, BufferLocation);
				});
		}


		void  IASetIndexBuffer(
			const D3D12_INDEX_BUFFER_VIEW* pView)
		{
			push_opt(pView);

			tasks.emplace_back([this]()
				{
					const D3D12_INDEX_BUFFER_VIEW* pView;
					pop_opt(pView);

					compiled.m_commandList->IASetIndexBuffer(pView);
				});
		}

		void  IASetVertexBuffers(
			UINT StartSlot,
			UINT NumViews,
			const D3D12_VERTEX_BUFFER_VIEW* pViews)
		{
			push(StartSlot, NumViews);
			push_array(pViews, NumViews);


			tasks.emplace_back([this]()
				{		UINT StartSlot;
			UINT NumViews;
			const D3D12_VERTEX_BUFFER_VIEW* pViews;

			pop(StartSlot, NumViews);
			pop_array(pViews, NumViews);

					compiled.m_commandList->IASetVertexBuffers(StartSlot, NumViews, pViews);
				});
		}

		void  SOSetTargets(
			UINT StartSlot,
			UINT NumViews,
			const D3D12_STREAM_OUTPUT_BUFFER_VIEW* pViews)
		{
			push(StartSlot, NumViews);
			push_array(pViews, NumViews);

			tasks.emplace_back([this]()
				{
					UINT StartSlot;
						UINT NumViews;
						 D3D12_STREAM_OUTPUT_BUFFER_VIEW* pViews;

						pop(StartSlot, NumViews);
						pop_array(pViews, NumViews);

					compiled.m_commandList->SOSetTargets(StartSlot, NumViews, pViews);
				});
		}

		void  OMSetRenderTargets(
			UINT NumRenderTargetDescriptors,
			const D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors,
			BOOL RTsSingleHandleToDescriptorRange,
			const D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor)
		{
			push(NumRenderTargetDescriptors, RTsSingleHandleToDescriptorRange);
			push_array(pRenderTargetDescriptors, NumRenderTargetDescriptors);
			push_opt(pDepthStencilDescriptor);

		
			tasks.emplace_back([this]()
				{
					UINT NumRenderTargetDescriptors;
					 D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors;
						BOOL RTsSingleHandleToDescriptorRange;
						 D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor;

						pop(NumRenderTargetDescriptors, RTsSingleHandleToDescriptorRange);
						pop_array(pRenderTargetDescriptors, NumRenderTargetDescriptors);
						pop_opt(pDepthStencilDescriptor);

				
					compiled.m_commandList->OMSetRenderTargets(NumRenderTargetDescriptors, pRenderTargetDescriptors, RTsSingleHandleToDescriptorRange, pDepthStencilDescriptor);
				});
		}

		void  ClearDepthStencilView(
			D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView,
			D3D12_CLEAR_FLAGS ClearFlags,
			FLOAT Depth,
			UINT8 Stencil,
			UINT NumRects,
			const D3D12_RECT* pRects)
		{
			push(DepthStencilView, ClearFlags, Depth, Stencil, NumRects);
			push_array(pRects, NumRects);


		

			tasks.emplace_back([this]()
				{
					D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView;
						D3D12_CLEAR_FLAGS ClearFlags;
						FLOAT Depth;
						UINT8 Stencil;
						UINT NumRects;
						 D3D12_RECT* pRects;

						pop(DepthStencilView, ClearFlags, Depth, Stencil, NumRects);
						pop_array(pRects, NumRects);

					compiled.m_commandList->ClearDepthStencilView(DepthStencilView, ClearFlags, Depth, Stencil, NumRects, pRects);
				});
		}

		void  ClearRenderTargetView(
			D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView,
			const FLOAT ColorRGBA[4],
			UINT NumRects,
			const D3D12_RECT* pRects)
		{
			push(RenderTargetView, NumRects);
			push_array(pRects, NumRects);
			push_array(ColorRGBA,4);


			tasks.emplace_back([this]()
				{
					D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView;
						FLOAT* ColorRGBA;
						UINT NumRects;
						 D3D12_RECT* pRects;
						pop(RenderTargetView, NumRects);
						pop_array(pRects, NumRects);
					pop_array(ColorRGBA,4);
					compiled.m_commandList->ClearRenderTargetView(RenderTargetView, ColorRGBA, NumRects, pRects);
				});
		}

		void  ClearUnorderedAccessViewUint(
			D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap,
			D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle,
			ID3D12Resource* pResource,
			const UINT Values[4],
			UINT NumRects,
			const D3D12_RECT* pRects)
		{
			push(ViewGPUHandleInCurrentHeap, ViewCPUHandle, pResource, NumRects);
			push_array(pRects, NumRects);
			push_array(Values,4);


			tasks.emplace_back([this]()
				{
					D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap;
						D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle;
						ID3D12Resource* pResource;
						 UINT* Values;
						UINT NumRects;
						 D3D12_RECT* pRects;

						pop(ViewGPUHandleInCurrentHeap, ViewCPUHandle, pResource, NumRects);
						pop_array(pRects, NumRects);
						pop_array(Values,4);

				
					compiled.m_commandList->ClearUnorderedAccessViewUint(ViewGPUHandleInCurrentHeap, ViewCPUHandle, pResource, Values, NumRects, pRects);
				});
		}

		void  ClearUnorderedAccessViewFloat(
			D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap,
			D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle,
			ID3D12Resource* pResource,
			const FLOAT Values[4],
			UINT NumRects,
			const D3D12_RECT* pRects)
		{
			push(ViewGPUHandleInCurrentHeap, ViewCPUHandle, pResource, NumRects);
			push_array(pRects, NumRects);
			push_array(Values,4);


			tasks.emplace_back([this]()
				{
					D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap;
						D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle;
						ID3D12Resource* pResource;
						FLOAT *Values;
						UINT NumRects;
						 D3D12_RECT* pRects;
						pop(ViewGPUHandleInCurrentHeap, ViewCPUHandle, pResource, NumRects);
						pop_array(pRects, NumRects);
					pop_array(Values,4);
				
					compiled.m_commandList->ClearUnorderedAccessViewFloat(ViewGPUHandleInCurrentHeap, ViewCPUHandle, pResource, Values, NumRects, pRects);
				});
		}

		void  DiscardResource(
			ID3D12Resource* pResource,
			const D3D12_DISCARD_REGION* pRegion)
		{
			push(pResource);
			push_opt(pRegion);

			tasks.emplace_back([this]()
				{
					ID3D12Resource* pResource;
						 D3D12_DISCARD_REGION* pRegion;
						pop(pResource);
						pop_opt(pRegion);
					compiled.m_commandList->DiscardResource(pResource, pRegion);
				});
		}

		void  BeginQuery(
			ID3D12QueryHeap* pQueryHeap,
			D3D12_QUERY_TYPE Type,
			UINT Index)
		{

			push(pQueryHeap, Type, Index);

			tasks.emplace_back([this]()
				{
					ID3D12QueryHeap* pQueryHeap;
					D3D12_QUERY_TYPE Type;
					UINT Index;

					pop(pQueryHeap, Type, Index);

				
					compiled.m_commandList->BeginQuery(pQueryHeap, Type, Index);
				});
		}

		void  EndQuery(
			ID3D12QueryHeap* pQueryHeap,
			D3D12_QUERY_TYPE Type,
			UINT Index)
		{
			push(pQueryHeap, Type, Index);


			tasks.emplace_back([this]()
				{
					ID3D12QueryHeap* pQueryHeap;
					D3D12_QUERY_TYPE Type;
					UINT Index;

					pop(pQueryHeap, Type, Index);
				
					compiled.m_commandList->EndQuery(pQueryHeap, Type, Index);
				});
		}

		void  ResolveQueryData(
			ID3D12QueryHeap* pQueryHeap,
			D3D12_QUERY_TYPE Type,
			UINT StartIndex,
			UINT NumQueries,
			ID3D12Resource* pDestinationBuffer,
			UINT64 AlignedDestinationBufferOffset)
		{
			push(pQueryHeap, Type, StartIndex, NumQueries, pDestinationBuffer, AlignedDestinationBufferOffset);

			tasks.emplace_back([this]()
				{

					ID3D12QueryHeap* pQueryHeap;
					D3D12_QUERY_TYPE Type;
					UINT StartIndex;
					UINT NumQueries;
					ID3D12Resource* pDestinationBuffer;
					UINT64 AlignedDestinationBufferOffset;
					pop(pQueryHeap, Type, StartIndex, NumQueries, pDestinationBuffer, AlignedDestinationBufferOffset);

					compiled.m_commandList->ResolveQueryData(pQueryHeap, Type, StartIndex, NumQueries, pDestinationBuffer, AlignedDestinationBufferOffset);
				});
		}

		void  SetPredication(
			ID3D12Resource* pBuffer,
			UINT64 AlignedBufferOffset,
			D3D12_PREDICATION_OP Operation)
		{
			push(pBuffer, AlignedBufferOffset, Operation);


			tasks.emplace_back([this]()
				{
					ID3D12Resource* pBuffer;
						UINT64 AlignedBufferOffset;
						D3D12_PREDICATION_OP Operation;
						pop(pBuffer, AlignedBufferOffset, Operation);

					compiled.m_commandList->SetPredication(pBuffer, AlignedBufferOffset, Operation);
				});
		}

		void  SetMarker(
			UINT Metadata,
			const void* pData,
			UINT Size)
		{
			push(Metadata, Size);
			push_array((const std::byte*)pData, Size);

			tasks.emplace_back([this]()
				{
			UINT Metadata;
						 std::byte* pData;
						UINT Size;
						pop(Metadata, Size);
						pop_array(pData, Size);

					compiled.m_commandList->SetMarker(Metadata, pData, Size);
				});
		}

		void  BeginEvent(
			UINT Metadata,
			const void* pData,
			UINT Size)
		{
			push(Metadata, Size);
			push_array((const std::byte*)pData, Size);

			tasks.emplace_back([this]()
				{
					UINT Metadata;
					 std::byte* pData;
					UINT Size;
					pop(Metadata, Size);
					pop_array(pData, Size);
				
					compiled.m_commandList->BeginEvent(Metadata, pData, Size);
				});
		}

		void  EndEvent(void)
		{
			tasks.emplace_back([this]()
				{
					compiled.m_commandList->EndEvent();
				});
		}

		void  ExecuteIndirect(
			ID3D12CommandSignature* pCommandSignature,
			UINT MaxCommandCount,
			ID3D12Resource* pArgumentBuffer,
			UINT64 ArgumentBufferOffset,
			ID3D12Resource* pCountBuffer,
			UINT64 CountBufferOffset)
		{

			push(pCommandSignature, MaxCommandCount, pArgumentBuffer, ArgumentBufferOffset, pCountBuffer, CountBufferOffset);

			tasks.emplace_back([this]()
				{
					ID3D12CommandSignature* pCommandSignature;
						UINT MaxCommandCount;
						ID3D12Resource* pArgumentBuffer;
						UINT64 ArgumentBufferOffset;
						ID3D12Resource* pCountBuffer;
						UINT64 CountBufferOffset;
						pop(pCommandSignature, MaxCommandCount, pArgumentBuffer, ArgumentBufferOffset, pCountBuffer, CountBufferOffset);

					compiled.m_commandList->ExecuteIndirect(pCommandSignature, MaxCommandCount, pArgumentBuffer, ArgumentBufferOffset, pCountBuffer, CountBufferOffset);
				});
		}

	};
}
