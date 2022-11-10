// CFW1StateSaver.h

#ifndef IncludeGuard__FW1_CFW1StateSaver
#define IncludeGuard__FW1_CFW1StateSaver


#include "FW1FontWrapper/Source/FW1FontWrapper.h"
namespace FW1FontWrapper {


// Saves all the states that can be changed when drawing a string
class CFW1StateSaver {
	// Public functions
	public:
		CFW1StateSaver();
		~CFW1StateSaver();
		
		HRESULT saveCurrentState();
		HRESULT restoreSavedState();
		void releaseSavedState();
	
	// Internal data
	private:
		
	private:
		CFW1StateSaver(const CFW1StateSaver&);
		CFW1StateSaver& operator=(const CFW1StateSaver&);
};


}// namespace FW1FontWrapper


#endif// IncludeGuard__FW1_CFW1StateSaver
