export module Graphics:RootSignature;
import :Enums;
import :Types; 
import :Device;
import stl.core;


template<class T>
void process_one_sig(Graphics::RootSignatureDesc& desc)
{
	if constexpr (HasSlot<T>)
		desc.remove(T::Slot::ID);
}



export
{

	namespace Graphics
	{

	

		template< class ...A>
		RootSignature::ptr create_global_signature(RootSignature& sig)
		{
			RootSignatureDesc desc = sig.get_desc();

			(process_one_sig<A>(desc), ...);

			return std::make_shared<RootSignature>(*Graphics::Device::get().get_hal_device(), desc);
		}



		class RootLayout :public RootSignature
		{
		public:
			using ptr = std::shared_ptr<RootLayout>;
			const Layouts layout;
			RootLayout(Device& device, const RootSignatureDesc& desc, Layouts layout) :RootSignature(*device.get_hal_device(), desc), layout(layout)
			{
			}
		};
	}
}