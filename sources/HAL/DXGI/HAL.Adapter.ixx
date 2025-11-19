export module HAL:Adapter;
import :Utils;
import <HAL.h>;
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
		Adapter(DXGI::Adapter native_adapter) :native_adapter(native_adapter)
		{
			

			native_adapter->GetDesc(&adapter_desc);
			Log::get() << "// Adapter: " << adapter_desc.Description << Log::endl;


			UINT ai = 0;
			IDXGIOutput* pOutput;

			while (native_adapter->EnumOutputs(ai, &pOutput) != DXGI_ERROR_NOT_FOUND)
			{
				DXGI_OUTPUT_DESC  oDesc;
				pOutput->GetDesc(&oDesc);
				++ai;

				Log::get() << "// Output: " << oDesc.DeviceName << Log::endl;

				pOutput->Release();
			}


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
			return native_factory;
		}
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
