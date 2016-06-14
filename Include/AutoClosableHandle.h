// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#pragma once

#include <Windows.h>

#include "AutoHandle.h"

//////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////// ClosableHandle
struct ClosableHandle
{
	typedef HANDLE handle_type;
	static	handle_type	DefaultHandle() { return INVALID_HANDLE_VALUE; }
	static	void		DestroyHandle(handle_type handle) { CloseHandle(handle); }
};
typedef AutoHandle<ClosableHandle> AutoClosableHandle;
