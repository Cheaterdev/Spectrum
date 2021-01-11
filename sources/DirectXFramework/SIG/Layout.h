#include "DX12/Samplers.h"

template<class T>
struct AutoGenSignatureDesc
{
	Render::RootSignatureDesc desc;


	template<class T>
	void process_one()
	{
		if constexpr (TableHasSRV<T>) desc[T::SRV_ID] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, -1, T::ID);
		if constexpr (TableHasUAV<T>) desc[T::UAV_ID] = Render::DescriptorTable(Render::DescriptorRange::UAV, Render::ShaderVisibility::ALL, 0, T::UAV, T::ID);
		if constexpr (TableHasSMP<T>) desc[T::SMP_ID] = Render::DescriptorTable(Render::DescriptorRange::SAMPLER, Render::ShaderVisibility::ALL, 0, T::SMP, T::ID);
		desc[T::CB_ID] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::ALL, T::ID);
		//	if constexpr (T::CB)	desc[T::CB_ID] = Render::DescriptorTable(Render::DescriptorRange::CBV, Render::ShaderVisibility::ALL, 0, T::CB, T::ID);

	}

	template< class ...A>
	void process(std::initializer_list< Render::Samplers::SamplerDesc> samplers)
	{

		(process_one<A>(), ...);
		for (auto& s : samplers)
		{
			desc.add_sampler(0, Render::ShaderVisibility::ALL, s);
		}
	}

	AutoGenSignatureDesc()
	{
		T::for_each(*this);

	}


	Render::RootLayout::ptr create_signature(Layouts layout)
	{
		return std::make_shared<Render::RootLayout>(desc, layout);
	}
};

template<class T>
void process_one(Render::RootSignatureDesc& desc)
{
	/*if constexpr (HasSRV<T>) 
		desc[T::Slot::SRV_ID] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, -1, T::Slot::ID);
	if constexpr (HasUAV<T>)
		desc[T::Slot::UAV_ID] = Render::DescriptorTable(Render::DescriptorRange::UAV, Render::ShaderVisibility::ALL, 0, T::Slot::UAV, T::Slot::ID);
	if constexpr (HasSMP<T>)
		desc[T::Slot::SMP_ID] = Render::DescriptorTable(Render::DescriptorRange::SAMPLER, Render::ShaderVisibility::ALL, 0, T::Slot::SMP, T::Slot::ID);*/
	if constexpr (HasCB<T>) 
		desc[T::Slot::CB_ID] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::ALL, T::Slot::ID);

}

template< class ...A>
Render::RootSignature::ptr create_local_signature()
{
	Render::RootSignatureDesc desc;

	(process_one<A>(desc), ...);
	
	return std::make_shared<Render::RootSignature>(desc, D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
}

