export module HAL:Adapter;
import :Utils;

import <d3d12/d3d12_includes.h>;

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
		Adapter(DXGI::Adapter native_adapter);

	public:
		using ptr = std::shared_ptr<Adapter>;


		const DXGI_ADAPTER_DESC& get_desc() const;
	private:

		friend class Adapters;

	};

	class Adapters :public Singleton<Adapters>
	{
		DXGI::Factory native_factory;

		friend class Singleton<Adapters>;
		Adapters();
	public:

		DXGI::Factory get_factory();
		void enumerate(auto f)
		{
			IDXGIAdapter4* padapter;

			uint i = 0;
			while (native_factory->EnumAdapterByGpuPreference (i++, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&padapter)) != DXGI_ERROR_NOT_FOUND)
			{
				DXGI::Adapter adapter;
				adapter.Attach(padapter);

				f(std::make_shared<Adapter>(adapter));
			}
		}
	};
}
