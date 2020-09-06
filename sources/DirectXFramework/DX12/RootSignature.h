#pragma once
namespace DX12
{

    enum class DescriptorRange : int
    {
        SRV = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        UAV = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
        CBV = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
        SAMPLER = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER
    };

#undef DOMAIN

    enum class ShaderVisibility : int
    {
        ALL = D3D12_SHADER_VISIBILITY_ALL,
        VERTEX = D3D12_SHADER_VISIBILITY_VERTEX,
        HULL = D3D12_SHADER_VISIBILITY_HULL,
        DOMAIN = D3D12_SHADER_VISIBILITY_DOMAIN,
        GEOMETRY = D3D12_SHADER_VISIBILITY_GEOMETRY,
        PIXEL = D3D12_SHADER_VISIBILITY_PIXEL
    };


    struct DescriptorParam
    {
        virtual ~DescriptorParam()
        {
        }

        virtual CD3DX12_ROOT_PARAMETER generate_native_param() = 0;
    };

    struct RootSignatureDesc;
	struct HandleTable;

    struct DescriptorTable//:public DescriptorParam
    {


        DescriptorRange range;
        ShaderVisibility visibility;
        UINT offset;
        UINT count;
        UINT space;
        DescriptorTable(DescriptorRange _range,
                        ShaderVisibility _visibility,
                        UINT _offset,
                        UINT _count, UINT space = 0): range(_range), space(space), visibility(_visibility), offset(_offset), count(_count)
        {
            //  native_range.Init(static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(range), count, offset);
        }


        /*   CD3DX12_DESCRIPTOR_RANGE& get_native()
           {
               return native_range;
           }
        private:
           CD3DX12_DESCRIPTOR_RANGE native_range;*/
    };

    struct DescriptorConstBuffer//:public DescriptorParam
    {
        DescriptorRange range;
        ShaderVisibility visibility;
        UINT offset;
		UINT space;

		DescriptorConstBuffer(UINT _offset, ShaderVisibility _visibility = ShaderVisibility::ALL,
			UINT space = 0
                             ) :  visibility(_visibility), offset(_offset), space(space)
        {
        }
    };
    struct DescriptorSRV//:public DescriptorParam
    {
        ShaderVisibility visibility;
        UINT offset;
        UINT space;


        DescriptorSRV(UINT _offset, ShaderVisibility _visibility = ShaderVisibility::ALL,
                      UINT space = 0) : visibility(_visibility), offset(_offset), space(space)
        {
        }
    };
    struct DescriptorUAV//:public DescriptorParam
    {
        ShaderVisibility visibility;
        UINT offset;
        UINT space;


        DescriptorUAV(UINT _offset, ShaderVisibility _visibility = ShaderVisibility::ALL,
                      UINT space = 0) : visibility(_visibility), offset(_offset), space(space)
        {
        }
    };
    struct DescriptorConstants//:public DescriptorParam
    {
        ShaderVisibility visibility;
        UINT count;
        UINT buffer_offset;
        DescriptorConstants(UINT _buffer_offset, UINT _count, ShaderVisibility _visibility = ShaderVisibility::ALL
                           ) : buffer_offset(_buffer_offset), visibility(_visibility), count(_count)
        {
        }
    };



    class RootSignature;
    struct RootSignatureDesc
    {
            friend class RootSignature;

			bool local = false;
			struct table_info
			{
				int count;
				bool fixed = true;
				DescriptorRange type;
			};
            struct helper
            {
                MyVariant& v;

				UINT index;


                void operator=(const DescriptorTable& table)
				{
					v = table;
                  /*  range.Init(static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(table.range), table.count, table.offset, table.space);
                    param.InitAsDescriptorTable(1, &range, static_cast<D3D12_SHADER_VISIBILITY>(table.visibility));
					if (table.range != DescriptorRange::SAMPLER)
					{
						tables[index].count = table.count;
						tables[index].type = table.range;
					}*/
                  
                }
                void operator=(const DescriptorConstBuffer& table)
				{
					v = table;
                 //   param.InitAsConstantBufferView(table.offset, table.space, static_cast<D3D12_SHADER_VISIBILITY>(table.visibility));
                }
                void operator=(const DescriptorSRV& table)
				{
					v = table;
                //    param.InitAsShaderResourceView(table.offset, table.space, static_cast<D3D12_SHADER_VISIBILITY>(table.visibility));
                }

                void operator=(const DescriptorUAV& table)
				{
					v = table;
                //    param.InitAsUnorderedAccessView(table.offset, table.space, static_cast<D3D12_SHADER_VISIBILITY>(table.visibility));
                }
                void operator=(const DescriptorConstants& table)
                {
                    v = table;
                  //  param.InitAsConstants(table.count, table.buffer_offset, 0, static_cast<D3D12_SHADER_VISIBILITY>(table.visibility));
                }

                helper(UINT index, MyVariant& v): index(index), v(v)
                {
                }

                helper(const helper& h) : index(h.index), v(h.v)
                {
                }
            };

            helper operator[](UINT i)
            {
            //     while (i >= ranges.size())
            //        ranges.emplace_back(new CD3DX12_DESCRIPTOR_RANGE);

                /* while (i >= helpers.size())
                  {
                      if (!ranges[i])
                          ranges[i].reset(new CD3DX12_DESCRIPTOR_RANGE);

                      helpers.emplace_back(i, parameters[i], *ranges[i], table_sizes);
                  }
                */
                return helper(i, parameters[i]);
            }

            void remove(UINT i)
            {
                parameters.erase(i);
            }

            const vector<D3D12_STATIC_SAMPLER_DESC>& samplers() const
            {
                return samplers_map;
            }

            void set_sampler(int i, int space, ShaderVisibility visibility,  D3D12_SAMPLER_DESC desc)
            {
                samplers_map.emplace_back();
                auto& sampler = samplers_map.back();
                sampler.AddressU = desc.AddressU;
                sampler.AddressV = desc.AddressV;
                sampler.AddressW = desc.AddressW;

                /*
                D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK	= 0,
                D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK	= ( D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK + 1 ) ,
                D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE	= ( D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK + 1 )
                */
                if (desc.BorderColor[0] == 0)
                    sampler.BorderColor = desc.BorderColor[3] == 0 ? D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK : D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
                else
                    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;

                sampler.ComparisonFunc = desc.ComparisonFunc;
                sampler.Filter = desc.Filter;
                sampler.MaxAnisotropy = desc.MaxAnisotropy;
                sampler.MaxLOD = desc.MaxLOD;
                sampler.MinLOD = desc.MinLOD;
                sampler.MipLODBias = desc.MipLODBias;
                sampler.RegisterSpace = space;
                sampler.ShaderRegister = i;
            }

            void add_sampler( int space, ShaderVisibility visibility, D3D12_SAMPLER_DESC desc)
            {
                set_sampler(samplers_map.size(), space, visibility, desc);
            }
            std::map<UINT, table_info> tables;

            UINT get_start_slot()
            {
                
            }


            public:
          //  std::map<int,CD3DX12_ROOT_PARAMETER> parameters;
          //  std::map<int,CD3DX12_DESCRIPTOR_RANGE> ranges;
            std::map<int, MyVariant> parameters;
            // std::vector<helper> helpers;
            std::vector<D3D12_STATIC_SAMPLER_DESC> samplers_map;
    };


    class RootSignature
    {
            ComPtr<ID3D12RootSignature> m_rootSignature;
            RootSignatureDesc desc;

            void finalize();
            template<class T>
            void set(int i, T v)
            {
                desc[i] = v;
            }
            template<class T, class ...Args>
            void set(int i, T v, Args...args)
            {
                set(i, v);
                set(++i, args...);
            }


        public:
            using ptr = std::shared_ptr<RootSignature>;

		
            virtual~RootSignature() = default;

            template <class ...Args>
            RootSignature(Args...args)
            {
                set(0, args...);
                finalize();
            }

            RootSignature(std::initializer_list<DescriptorTable> table);
            RootSignature(const RootSignatureDesc& desc, D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

            ComPtr<ID3D12RootSignature> get_native();
			void set_unfixed(int i)
			{
				desc.tables[i].fixed = false;
			}
            const RootSignatureDesc& get_desc() const
            {
                return desc;
            }


            template<class T>
            void process_one(RootSignatureDesc& desc)
            {
             
                if constexpr (HasCB<T>)
                    desc.remove(T::Slot::CB_ID);

            }


			template< class ...A>
			ptr create_global_signature()
			{
			//	RootSignatureDesc desc = get_desc();

				(process_one<A>(desc), ...);

				return std::make_shared<Render::RootSignature>(desc, D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_NONE);
			}



    };


    class RootLayout:public RootSignature
    {
		
      
	public:
		using ptr = std::shared_ptr<RootLayout>;
        const Layouts layout;
        RootLayout(const RootSignatureDesc& desc, Layouts layout):RootSignature(desc), layout(layout)
        {


        }
    };


    /*
    class RootLayoutManager
    {
        std::vector<Layouts> slots; 
    public:

        void set_layout(RootLayout::ptr &layout)
        {
            for (auto& e : layout->get_desc().tables)
            {
                slots[]
            }
        }
    };*/
	template<class T>
	class RootSignatureTyped: public RootSignature
	{
	public:

		using ptr = std::shared_ptr<RootSignatureTyped<T>>;


		RootSignatureTyped(const RootSignatureDesc& desc, D3D12_ROOT_SIGNATURE_FLAGS flags= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT):RootSignature(desc, flags)
		{

		}
	};

}
