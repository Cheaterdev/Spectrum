export module HAL:Adapter;
import :Utils;

import d3d12;

import Core;

export namespace HAL
{

	struct AdapterDesc
	{

	};

	class Adapter
	{
	public://///////////
		DXGI::Adapter native_adapter;
		DXGI_ADAPTER_DESC adapter_desc;
		Adapter(DXGI::Adapter native_adapter) :native_adapter(native_adapter)
		{

		}
	public:
		using ptr = std::shared_ptr<Adapter>;


		const DXGI_ADAPTER_DESC& get_desc() const {
			return adapter_desc;
		}
	private:

		friend class Adapters;

	};

	class Adapters :public Singleton<Adapters>
	{
		DXGI::Factory native_factory;

		friend class Singleton<Adapters>;
		Adapters()
		{
			CreateDXGIFactory2(0, IID_PPV_ARGS(&native_factory));
		}
	public:

		DXGI::Factory get_factory() {
			return native_factory;	}
		void enumerate(auto f)
		{
			IDXGIAdapter3* padapter;

			uint i = 0;
			while (native_factory->EnumAdapters(i++, reinterpret_cast<IDXGIAdapter**>(&padapter)) != DXGI_ERROR_NOT_FOUND)
			{
				DXGI::Adapter adapter;
				adapter.Attach(padapter);



				f(std::make_shared<Adapter>(adapter));
			}
		}
	};
}
