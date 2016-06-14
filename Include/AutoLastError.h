// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#pragma once

#include "AutoHandle.h"

#include <Windows.h>

//////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////// LastErrorHandle
struct LastErrorHandle
{
	typedef DWORD handle_type;
	static	handle_type	DefaultHandle() { return GetLastError(); }
	static	void		DestroyHandle(handle_type handle) { SetLastError(handle); }
};

typedef AutoHandle<LastErrorHandle> AutoLastError;
