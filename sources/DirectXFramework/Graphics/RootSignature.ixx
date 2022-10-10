export module Graphics:RootSignature;
import :Enums;
import :Types;
import :Device;
import stl.core;

export
{
	namespace Graphics
	{
		using DescriptorConstBuffer = HAL::DescriptorConstBuffer;
		using RootSignatureDesc = HAL::RootSignatureDesc;
		using RootSignatureType = HAL::RootSignatureType;

		using RootSignature = HAL::RootSignature;


		class RootLayout :public RootSignature
		{
			Device& device;

			template<class T>
			void process_one_sig(Graphics::RootSignatureDesc& desc) const
			{
				if constexpr (HasSlot<T>)
					desc.remove(T::Slot::ID);
			}

		public:
			using ptr = std::shared_ptr<RootLayout>;
			const Layouts layout;
			RootLayout(Device& device, const RootSignatureDesc& desc, Layouts layout) :RootSignature(device, desc), device(device), layout(layout)
			{
			}

			template< class ...A>
			RootSignature::ptr create_global_signature() const
			{
				RootSignatureDesc desc = get_desc();

				(process_one_sig<A>(desc), ...);

				return std::make_shared<RootSignature>(device, desc);
			}

		};
	}
}