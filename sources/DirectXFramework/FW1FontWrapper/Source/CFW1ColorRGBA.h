// CFW1ColorRGBA.h

#ifndef IncludeGuard__FW1_CFW1ColorRGBA
#define IncludeGuard__FW1_CFW1ColorRGBA

#include "CFW1Object.h"


namespace FW1FontWrapper {


// A color
class CFW1ColorRGBA : public CFW1Object<IFW1ColorRGBA> {
	public:
		// IUnknown
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
		
		// IFW1Color32
		virtual void STDMETHODCALLTYPE SetColor(float4 color);

		
		virtual float4 STDMETHODCALLTYPE GetColor();
	
	// Public functions
	public:
		CFW1ColorRGBA();
		
	// Internal functions
	private:
		virtual ~CFW1ColorRGBA();
	
	// Internal data
	private:
		float4 color;
};


}// namespace FW1FontWrapper


#endif// IncludeGuard__FW1_CFW1ColorRGBA
