// CFW1Object.h

#ifndef IncludeGuard__FW1_CFW1Object
#define IncludeGuard__FW1_CFW1Object

#include <Core_defs.h>
#include "FW1FontWrapper/Source/FW1FontWrapper.h"

namespace FW1FontWrapper {


// Helper baseclass to avoid writing IUnknown and IFW1Object implementations once per class
template<class IBase>
class CFW1Object : public IBase {
	public:
		// IUnknown
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(const IID& riid, void **ppvObject) = 0 {
			if(ppvObject == nullptr)
				return E_INVALIDARG;

			if(riid == __uuidof(IUnknown)) {
				*ppvObject = static_cast<IUnknown*>(this);
				AddRef();
				return S_OK;
			}
			else if(riid == __uuidof(IFW1Object)) {
				*ppvObject = static_cast<IFW1Object*>(this);
				AddRef();
				return S_OK;
			}

			*ppvObject = nullptr;
			return E_NOINTERFACE;
		}

		virtual unsigned long STDMETHODCALLTYPE AddRef() {
			return ++m_cRefCount;
		}

		virtual unsigned long STDMETHODCALLTYPE Release() {
			unsigned long newCount = --m_cRefCount;

			if(newCount == 0)
				delete this;

			return newCount;
		}

		// IFW1Object
		virtual HRESULT STDMETHODCALLTYPE GetFactory(IFW1Factory **ppFW1Factory) {
			if(ppFW1Factory == nullptr)
				return E_INVALIDARG;

			m_pFW1Factory->AddRef();
			*ppFW1Factory = m_pFW1Factory;

			return S_OK;
		}

	// Internal functions
	protected:
		CFW1Object() :
			m_cRefCount(1),

			m_pFW1Factory(nullptr)
		{
		}

		virtual ~CFW1Object() {
			if(m_pFW1Factory != nullptr)
				m_pFW1Factory->Release();
		}

		HRESULT initBaseObject(IFW1Factory *pFW1Factory) {
			if(pFW1Factory == nullptr)
				return E_INVALIDARG;

			pFW1Factory->AddRef();
			m_pFW1Factory = pFW1Factory;

			return S_OK;
		}

	// Internal data
	protected:
		IFW1Factory					*m_pFW1Factory;

	private:
		std::atomic<unsigned long>	m_cRefCount;

	private:
		CFW1Object(const CFW1Object&);
		CFW1Object& operator=(const CFW1Object&);
};


}// namespace FW1FontWrapper


#endif// IncludeGuard__FW1_CFW1Object
