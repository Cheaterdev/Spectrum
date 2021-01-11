// CFW1StateSaver.cpp

#include "pch.h"

#include "CFW1StateSaver.h"

#define SAFE_RELEASE(pObject) { if(pObject) { (pObject)->Release(); (pObject) = NULL; } }


namespace FW1FontWrapper {


// Construct
CFW1StateSaver::CFW1StateSaver() :
	m_savedState(false)
{

}


// Destruct
CFW1StateSaver::~CFW1StateSaver() {
	releaseSavedState();
}


// Save all states that are changed by the font-wrapper when drawing a string
HRESULT CFW1StateSaver::saveCurrentState() {
	if(m_savedState)
		releaseSavedState();

	return S_OK;
}


// Restore state
HRESULT CFW1StateSaver::restoreSavedState() {
	if(!m_savedState)
		return E_FAIL;

	return S_OK;
}


// Release state
void CFW1StateSaver::releaseSavedState() {
	m_savedState = false;
}


}// namespace FW1FontWrapper
