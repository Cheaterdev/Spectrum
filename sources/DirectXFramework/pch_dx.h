#pragma once

import d3d12;

HRESULT device_fail();
HRESULT test(HRESULT hr, std::string str = "");

#ifdef  DEBUG
#define TEST(x)\
	assert(SUCCEEDED(x))
#else
#define TEST(x)\
	([&](){\
	HRESULT hr = x; \
	if(hr==0x887a0005) device_fail(); \
	 test(hr, STRINGIZE(x)); \
if (FAILED(hr)) \
	{__debugbreak(); \
	assert(false); }\
	return hr; \
	})()

#endif //  DEBUG


namespace DX12
{
	
}
namespace Render
{
	using namespace DX12;
}





