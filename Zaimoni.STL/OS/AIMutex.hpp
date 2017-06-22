// AIMutex.hpp
// header for AIMutex
// (C)2009 Kenneth Boyd, license: MIT.txt

#ifndef ZAIMONI_OS_AIMUTEX_HPP
#define ZAIMONI_OS_AIMUTEX_HPP 1

#if defined(_WIN32)
#include <WINDOWS.H>
#else
#error("Error: headers for AIMutex not implemented.")
#endif

#if defined(_WIN32)
class AIMutex
{
private:
	HANDLE TheMutex;

	AIMutex(const AIMutex& source);
	const AIMutex& operator=(const AIMutex& source);
public:
	inline AIMutex() : TheMutex(CreateMutex(NULL,FALSE,NULL)) {};
	inline AIMutex(LPCTSTR Name) : TheMutex(CreateMutex(NULL,FALSE,Name)) {};
	inline AIMutex(LPSECURITY_ATTRIBUTES Security) : TheMutex(CreateMutex(Security,FALSE,NULL)) {};
	inline AIMutex(LPSECURITY_ATTRIBUTES Security, LPCTSTR Name) : TheMutex(CreateMutex(Security,FALSE,Name)) {};
	inline ~AIMutex() {CloseHandle(TheMutex);};

	// State information
	inline bool IsBadMutex(void) const	{return NULL == TheMutex;}

	// lock the mutex
	inline DWORD Lock(void)				{return WaitForSingleObject(TheMutex,INFINITE);};
	inline DWORD Lock(DWORD milliseconds)	{return WaitForSingleObject(TheMutex,milliseconds);};

	// unlock the mutex
	inline BOOL UnLock(void) {return ReleaseMutex(TheMutex);};
};
#endif

#endif // ZAIMONI_OS_AIMUTEX_HPP
