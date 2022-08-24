// CFW1ColorRGBAInterface.cpp

#include "pch_dx.h"

#include "CFW1ColorRGBA.h"


namespace FW1FontWrapper {


// Query interface
HRESULT STDMETHODCALLTYPE CFW1ColorRGBA::QueryInterface(REFIID riid, void **ppvObject) {
	if(ppvObject == NULL)
		return E_INVALIDARG;
	
	if(IsEqualIID(riid, __uuidof(IFW1ColorRGBA))) {
		*ppvObject = static_cast<IFW1ColorRGBA*>(this);
		AddRef();
		return S_OK;
	}
	
	return CFW1Object::QueryInterface(riid, ppvObject);
}


// Set the color
void STDMETHODCALLTYPE CFW1ColorRGBA::SetColor(float4 Color) {
	color = Color;
}



// Get the color
float4 STDMETHODCALLTYPE CFW1ColorRGBA::GetColor() {
	return color;
}


}// namespace FW1FontWrapper
