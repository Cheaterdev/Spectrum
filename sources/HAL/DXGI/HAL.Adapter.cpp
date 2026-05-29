module HAL:Adapter;
import :Utils;

import d3d12;
import Core;

namespace HAL
{
	Adapter::Adapter(DXGI::Adapter native_adapter) : native_adapter(native_adapter)
	{
		native_adapter->GetDesc(&adapter_desc);
		Log::get() << "// Adapter: " << adapter_desc.Description << Log::endl;

		UINT ai = 0;
		IDXGIOutput* pOutput;

		while (native_adapter->EnumOutputs(ai, &pOutput) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_OUTPUT_DESC oDesc;
			pOutput->GetDesc(&oDesc);
			++ai;

			Log::get() << "// Output: " << oDesc.DeviceName << Log::endl;

			pOutput->Release();
		}
	}

	const DXGI_ADAPTER_DESC& Adapter::get_desc() const { return adapter_desc; }

	Adapters::Adapters()
	{
		CreateDXGIFactory2(0, IID_PPV_ARGS(&native_factory));
	}

	DXGI::Factory Adapters::get_factory() { return native_factory; }
}
