// mutex.cpp
// implementation of OS-hiding mutex
// (C)2009 Kenneth Boyd, license: MIT.txt

#include "../mutex.hpp"
#include <WINDOWS.H>

namespace zaimoni {
namespace OS {

// HANDLE is a typedef for a POD-type

mutex::mutex()
:	_raw_mutex(calloc(1,sizeof(HANDLE)))
{
	if (NULL==_raw_mutex) exit(EXIT_FAILURE);
	*reinterpret_cast<HANDLE*>(_raw_mutex) = CreateMutex(NULL,FALSE,NULL);
	if (NULL==*reinterpret_cast<HANDLE*>(_raw_mutex)) exit(EXIT_FAILURE);
}

mutex::~mutex()
{
	CloseHandle(*reinterpret_cast<HANDLE*>(_raw_mutex));
	free(_raw_mutex);
}

void mutex::lock()
{
	WaitForSingleObject(*reinterpret_cast<HANDLE*>(_raw_mutex),INFINITE);
}

bool mutex::unlock()
{
	return ReleaseMutex(*reinterpret_cast<HANDLE*>(_raw_mutex));
}

}	// namespace OS
}	// namespace zaimoni


