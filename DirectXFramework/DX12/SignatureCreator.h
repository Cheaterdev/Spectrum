#pragma once

namespace DX12
{
	namespace Descriptors
	{

		namespace Realtime
		{

			template <int slot, ShaderVisibility v, int offset, int count, int space = 0, class Type = Vector<vecN_t<UINT32, count>>>
			struct Constants;

			template <int slot, ShaderVisibility v, int offset, int space = 0>
			struct ConstBuffer;

			template <int slot, ShaderVisibility v, int offset, int space = 0>
			struct SRV;

			template <int slot, ShaderVisibility v, int offset, int space = 0>
			struct UAV;

			template <int slot, ShaderVisibility v, DescriptorRange range, int offset, int count, int space = 0>
			struct Table;

			template<int slot, ShaderVisibility v, int offset, int space = 0>
			struct Sampler;

		}
		namespace Creation
		{

			template <int slot, ShaderVisibility v, int offset, int count, int space = 0, class Type = Vector<vecN_t<UINT32, count>>>
			struct Constants;

			template <int slot, ShaderVisibility v, int offset, int space = 0>
			struct ConstBuffer;

			template <int slot, ShaderVisibility v, int offset, int space = 0>
			struct SRV;

			template <int slot, ShaderVisibility v, int offset, int space = 0>
			struct UAV;

			template <int slot, ShaderVisibility v, DescriptorRange range, int offset, int count, int space = 0>
			struct Table;

			template<int slot, ShaderVisibility v, int offset, int space = 0>
			struct Sampler;
		}
	}
	struct Signature;
	struct SignatureCreator;

	struct offset
	{
		 int value;
	};

	constexpr  offset operator""_offset(unsigned long long  v)
	{
		return offset{ (int)v };
	}


	struct SRV2
	{
		const int slot;

		DX12::SignatureDataSetter* context;

		void set(Signature* sig)
		{

		}

		void set(SignatureCreator* sig)
		{

		}

		void set(ShaderVisibility v)
		{

		}

		void set(offset off)
		{

		}

		template<class ...Args>
		SRV2(int _slot, Args...args) :slot(_slot)
		{
			(set(args), ...);
		}

	};


	struct SignatureCreator
	{

		using SRV_2 = SRV2;

		template <int slot, ShaderVisibility v, int offset, int count, int space = 0, class Type = Vector<vecN_t<UINT32, count>>>
		using Constants = typename  Descriptors::Creation::Constants <slot, v, offset, count, space, Type>;

		template <int slot, ShaderVisibility v, int offset, int space = 0>
		using ConstBuffer = typename  Descriptors::Creation::ConstBuffer <slot, v, offset, space>;

		template <int slot, ShaderVisibility v, int offset, int space = 0>
		using SRV = typename  Descriptors::Creation::SRV <slot, v, offset, space>;

		template <int slot, ShaderVisibility v, int offset, int space = 0>
		using UAV = typename  Descriptors::Creation::UAV <slot, v, offset, space>;

		template <int slot, ShaderVisibility v, DescriptorRange range, int offset, int count, int space = 0 >
		using Table = typename  Descriptors::Creation::Table <slot, v, range, offset, count, space>;

		template<int slot, ShaderVisibility v, int offset, int space = 0>
		using Sampler = typename  Descriptors::Creation::Sampler <slot, v, offset, space>;


		RootSignatureDesc desc;

		SignatureCreator() = default;



		RootSignature::ptr create_root();

		template<class T>
		typename RootSignatureTyped<T>::ptr create_typed_root()
		{
			return std::make_shared<RootSignatureTyped<T>>(desc);
		}
		
	};


	struct Signature
	{

		using SRV_2 = SRV2;

		template <int slot, ShaderVisibility v, int offset, int count, int space = 0, class Type = Vector<vecN_t<UINT32, count>>>
		using Constants = typename  Descriptors::Realtime::Constants <slot, v, offset, count, space, Type>;

		template <int slot, ShaderVisibility v, int offset, int space = 0>
		using ConstBuffer = typename  Descriptors::Realtime::ConstBuffer <slot, v, offset, space>;

		template <int slot, ShaderVisibility v, int offset, int space = 0>
		using SRV = typename  Descriptors::Realtime::SRV <slot, v, offset, space>;

		template <int slot, ShaderVisibility v, int offset, int space = 0>
		using UAV = typename  Descriptors::Realtime::UAV <slot, v, offset, space>;

		template <int slot, ShaderVisibility v, DescriptorRange range, int offset, int count, int space = 0 >
		using Table = typename  Descriptors::Realtime::Table <slot, v, range, offset, count, space>;

		template<int slot, ShaderVisibility v, int offset, int space = 0>
		using Sampler = typename  Descriptors::Realtime::Sampler <slot, v, offset, space>;


		DX12::SignatureDataSetter* context = nullptr;

		Signature(DX12::SignatureDataSetter* context);
		Signature(const Signature&) = delete;
		Signature(Signature&&) = delete;

	};


	struct Indirect
	{
		using SRV_2 = SRV2;


		template <int slot, ShaderVisibility v, int offset, int count, int space = 0, class Type = Vector<vecN_t<UINT32, count>>>
		using Constants = typename  Descriptors::Realtime::Constants <slot, v, offset, count, space, Type>::IndirectType;

		template <int slot, ShaderVisibility v, int offset, int space = 0>
		using ConstBuffer = typename  Descriptors::Realtime::ConstBuffer <slot, v, offset, space>::IndirectType;

		template <int slot, ShaderVisibility v, int offset, int space = 0>
		using SRV = typename  Descriptors::Realtime::SRV <slot, v, offset, space>::IndirectType;

		template <int slot, ShaderVisibility v, int offset, int space = 0>
		using UAV = typename  Descriptors::Realtime::UAV <slot, v, offset, space>::IndirectType;

		template <int slot, ShaderVisibility v, DescriptorRange range, int offset, int count, int space = 0 >
		using Table = int;

		template<int slot, ShaderVisibility v, int offset, int space = 0>
		using Sampler = int;


		Indirect() = delete;

	};



	namespace Descriptors
	{

		namespace Realtime
		{


			template <int slot, ShaderVisibility v, int offset, int count, int space = 0, class Type = Vector<vecN_t<UINT32, count>>>
			struct Constants
			{
			/*	operator int()
				{
					return slot;
				}
				*/
			
				template<class... Args>
				void set(Args...args)
				{
					context->set_constants(slot, args...);
				}


				using IndirectType = Type;

				static	D3D12_INDIRECT_ARGUMENT_DESC create_indirect(int const_offset = 0, int values_to_set = count)
				{
					D3D12_INDIRECT_ARGUMENT_DESC desc;

					desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
					desc.Constant.RootParameterIndex = slot;
					desc.Constant.DestOffsetIn32BitValues = const_offset;
					desc.Constant.Num32BitValuesToSet = values_to_set;

					return desc;
				}


				Constants(Signature* sig) :context(const_cast<DX12::SignatureDataSetter*>(sig->context))
				{

				}

				DX12::SignatureDataSetter*& context;
			};

			template<UINT slot>
			struct TableSetter
			{
				DX12::SignatureDataSetter& context;
				UINT offset;

				TableSetter(DX12::SignatureDataSetter& context, UINT offset):context(context), offset(offset)
				{

				}
				void operator=(const Handle& handle)
				{
					context.set_dynamic(slot, offset, handle);
				}
				void operator=(const HandleTable& table)
				{
					context.set_dynamic(slot, offset, table);
				}

			};

			template <int slot, ShaderVisibility v, DescriptorRange range, int offset, int count, int space = 0>
			struct Table
			{

				static constexpr bool has_value(int i)
				{
					return i >= offset && i < offset + count;
				}
				operator int()
				{
					return slot;
				}


				void operator=(const HandleTable& table) {
					context->set(slot, table);
				}
				void operator=(const Handle& handle) {
					context->set(slot, handle);
				}

				void operator=(std::vector<Handle>& handles) {
					context->set(slot, handles);
				}


				TableSetter<slot> operator[](UINT offset)
				{
					return TableSetter<slot>( *context , offset );
				}

				HandleTable create_handle_table()
				{
					if constexpr (range == DescriptorRange::UAV)
						return Render::DescriptorHeapManager::get().get_csu_static()->create_table(3);
					else
						return Render::DescriptorHeapManager::get().get_srv_static()->create_table(3);
				}



				Table(Signature* sig) :context(const_cast<DX12::SignatureDataSetter*>(sig->context))
				{

				}


				DX12::SignatureDataSetter*& context;
			};

			template <int slot, ShaderVisibility v, int offset, int space = 0>
			struct ConstBuffer
			{
			/*	operator int()
				{
					return slot;
				}
				*/
				using IndirectType = D3D12_GPU_VIRTUAL_ADDRESS;

				static constexpr bool has_value(int i)
				{
					return false;// i >= offset && i < offset + count;
				}

				void operator = (const D3D12_GPU_VIRTUAL_ADDRESS& address)
				{
					context->set_const_buffer(slot, address);
				}
				void operator = (std::shared_ptr<GPUBuffer>& buff)
				{
					context->set_const_buffer(slot, buff);
				}

				void operator = (const FrameResources::UploadInfo& info)
				{
					context->set_const_buffer(slot, info);
				}

				void operator = (FrameResource& res)
				{
					context->set_const_buffer(slot, res);
				}


				template<class V>
				void operator=(std::vector<V> &data)
				{
					context->set_const_buffer_raw(slot, data.data(), sizeof(V)*data.size());
				}
				template<class V,int C>
				void operator=(std::array<V, C>& data)
				{
					context->set_const_buffer_raw(slot, data.data(), sizeof(V) * C);
				}

			/*	template<class V>
				void operator=(const V& data)
				{
					context->set_const_buffer_raw(slot, &data, sizeof(V));
				}*/

				template<class V>
				void set_raw(const V* data, size_t size)
				{
					context->set_const_buffer_raw(slot, data, sizeof(V)*size);
				}

				template<class V>
				void set_raw(const V& data)
				{
					context->set_const_buffer_raw(slot, &data, sizeof(V));
				}

				static D3D12_INDIRECT_ARGUMENT_DESC create_indirect()
				{
					D3D12_INDIRECT_ARGUMENT_DESC desc;

					desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
					desc.ConstantBufferView.RootParameterIndex = slot;
					return desc;
				}



				ConstBuffer(Signature* sig) :context(const_cast<DX12::SignatureDataSetter*>(sig->context))
				{

				}


				DX12::SignatureDataSetter*& context;
			};



			template <int slot, ShaderVisibility v, int offset, int space = 0>
			struct SRV
			{
			/*	operator int()
				{
					return slot;
				}
				*/

				using IndirectType = D3D12_GPU_VIRTUAL_ADDRESS;


				static D3D12_INDIRECT_ARGUMENT_DESC create_indirect()
				{
					D3D12_INDIRECT_ARGUMENT_DESC desc;

					desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
					desc.ShaderResourceView.RootParameterIndex = slot;

					return desc;
				}


				void operator=(D3D12_GPU_VIRTUAL_ADDRESS address)
				{
					context->set_srv(slot, address);
				}

				void operator=(FrameResource& info)
				{
					context->set_srv(slot, info);
				}



				template<class T>
				void operator=(const std::vector<T>& data)
				{
					context->set_srv(slot, data);
				}


				SRV(Signature* sig) :context(const_cast<DX12::SignatureDataSetter*>(sig->context))
				{

				}


				DX12::SignatureDataSetter*& context;
			};

			template <int slot, ShaderVisibility v, int offset, int space = 0>
			struct UAV
			{
			/*	operator int()
				{
					return slot;
				}*/

				using IndirectType = D3D12_GPU_VIRTUAL_ADDRESS;

				static D3D12_INDIRECT_ARGUMENT_DESC create_indirect()
				{
					D3D12_INDIRECT_ARGUMENT_DESC desc;

					desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW;
					desc.ShaderResourceView.RootParameterIndex = slot;

					return desc;
				}


				void operator=(D3D12_GPU_VIRTUAL_ADDRESS address)
				{
					context->set_uav(slot, address);
				}



				UAV(Signature* sig) :context(const_cast<DX12::SignatureDataSetter*>(sig->context))
				{

				}


				DX12::SignatureDataSetter*& context;
			};


			template<int slot, ShaderVisibility v, int offset, int space = 0>
			struct Sampler
			{

				Sampler(D3D12_SAMPLER_DESC s, Signature* creator) 
				{

				}

				operator int()
				{
					return slot;
				}

			};
		}
		namespace Creation
		{

			template <int slot, ShaderVisibility v, int offset, int count, int space = 0, class Type = Vector<vecN_t<UINT32, count>>>
			struct Constants
			{

				using IndirectType = Type;

				static	D3D12_INDIRECT_ARGUMENT_DESC create_indirect(int const_offset = 0, int values_to_set = count)
				{
					D3D12_INDIRECT_ARGUMENT_DESC desc;

					desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
					desc.Constant.RootParameterIndex = slot;
					desc.Constant.DestOffsetIn32BitValues = const_offset;
					desc.Constant.Num32BitValuesToSet = values_to_set;

					return desc;
				}

				Constants(SignatureCreator* creator)
				{
					creator->desc[slot] = DescriptorConstants(offset, count, v);
				}

			};

			template <int slot, ShaderVisibility v, DescriptorRange range, int offset, int count, int space = 0>
			struct Table
			{

				static constexpr bool has_value(int i)
				{
					return i >= offset && i < offset + count;
				}

				Table(SignatureCreator* creator)
				{
					creator->desc[slot] = DescriptorTable(range, v, offset, count, space);
				}

			};

			template <int slot, ShaderVisibility v, int offset, int space = 0>
			struct ConstBuffer
			{

				using IndirectType = D3D12_GPU_VIRTUAL_ADDRESS;

				static constexpr bool has_value(int i)
				{
					return false;// i >= offset && i < offset + count;
				}


				static D3D12_INDIRECT_ARGUMENT_DESC create_indirect()
				{
					D3D12_INDIRECT_ARGUMENT_DESC desc;

					desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
					desc.ConstantBufferView.RootParameterIndex = slot;
					return desc;
				}




				ConstBuffer(SignatureCreator* creator)
				{
					creator->desc[slot] = DescriptorConstBuffer(offset, v);
				}

			};



			template <int slot, ShaderVisibility v, int offset, int space = 0>
			struct SRV
			{

				using IndirectType = D3D12_GPU_VIRTUAL_ADDRESS;


				static D3D12_INDIRECT_ARGUMENT_DESC create_indirect()
				{
					D3D12_INDIRECT_ARGUMENT_DESC desc;

					desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
					desc.ShaderResourceView.RootParameterIndex = slot;

					return desc;
				}





				SRV(SignatureCreator* creator)
				{
					creator->desc[slot] = DescriptorSRV(offset, v, space);
				}


			};

			template <int slot, ShaderVisibility v, int offset, int space = 0>
			struct UAV
			{


				using IndirectType = D3D12_GPU_VIRTUAL_ADDRESS;

				static D3D12_INDIRECT_ARGUMENT_DESC create_indirect()
				{
					D3D12_INDIRECT_ARGUMENT_DESC desc;

					desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW;
					desc.ShaderResourceView.RootParameterIndex = slot;

					return desc;
				}




				UAV(SignatureCreator* creator)
				{
					creator->desc[slot] =DescriptorUAV(offset, v, space);
				}


			};


			template<int slot, ShaderVisibility v, int offset, int space = 0>
			struct Sampler
			{
				D3D12_SAMPLER_DESC s;

				Sampler(D3D12_SAMPLER_DESC s, SignatureCreator* creator) :s(s)
				{
					creator->desc.set_sampler(slot, space, v, s);
				}

			};

		}
		struct IndexBuffer
		{
			using IndirectType = D3D12_INDEX_BUFFER_VIEW;


			static D3D12_INDIRECT_ARGUMENT_DESC create_indirect()
			{
				D3D12_INDIRECT_ARGUMENT_DESC desc;

				desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;

				return desc;
			}


			IndexBuffer() = default;



		};
		static IndexBuffer index_buffer;


		struct DrawIndirect
		{

			using IndirectType = D3D12_DRAW_INDEXED_ARGUMENTS;




			static D3D12_INDIRECT_ARGUMENT_DESC create_indirect()
			{
				D3D12_INDIRECT_ARGUMENT_DESC desc;

				desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

				return desc;
			}

		};

		static DrawIndirect draw_indirect;

		struct Dispatch
		{


			using IndirectType = D3D12_INDEX_BUFFER_VIEW;

			static D3D12_INDIRECT_ARGUMENT_DESC create_indirect()
			{
				D3D12_INDIRECT_ARGUMENT_DESC desc;

				desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

				return desc;
			}

		};

	}


	struct desc_creator
	{
		RootSignatureDesc  desc;
		//int i = 0;


		template<class T>
		desc_creator& operator&(T& obj)
		{


			obj.process(desc);

			return *this;
		}


	};


	template< template<typename> typename Type>
	struct SignatureTypes
	{
		using Creator = Type<DX12::SignatureCreator>;
		using Signature = Type<DX12::Signature>;


		using Pipeline = DX12::PipelineStateTyped<SignatureTypes<Type>>;
		using RootSignature = DX12::RootSignatureTyped<SignatureTypes<Type>>;

		static DX12::RootSignature::ptr create_root()
		{
			return Creator().create_root();
		}

		static typename RootSignatureTyped<SignatureTypes>::ptr create_typed_root()
		{
			return Creator().create_typed_root<SignatureTypes<Type>>();
		}


		SignatureTypes() = delete;
	};


}