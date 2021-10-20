// CFW1StateSaver.cpp

#include "pch_dx.h"

#include "CFW1StateSaver.h"

#define SAFE_RELEASE(pObject) { if(pObject) { (pObject)->Release(); (pObject) = NULL; } }


namespace FW1FontWrapper {


// Construct
CFW1StateSaver::CFW1StateSaver() 
{
	
}


// Destruct
CFW1StateSaver::~CFW1StateSaver() {
	releaseSavedState();
}


// Save all states that are changed by the font-wrapper when drawing a string
HRESULT CFW1StateSaver::saveCurrentState() {

	
	return S_OK;
}


// Restore state
HRESULT CFW1StateSaver::restoreSavedState() {

	return S_OK;
}


// Release state
void CFW1StateSaver::releaseSavedState() {

}


}// namespace FW1FontWrapper
