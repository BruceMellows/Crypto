// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#pragma once

#include <Windows.h>
#include "AutoHandle.h"
#include "AutoCriticalSection.h"

//////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////// CryptHashHandle
struct EnterCriticalSectionHandle
{
	typedef LPCRITICAL_SECTION handle_type;
	static	handle_type	DefaultHandle() { return 0; }
	static	void		DestroyHandle(handle_type handle) { LeaveCriticalSection(handle); }
};
typedef AutoHandle<EnterCriticalSectionHandle> AutoEnterCriticalSection;

// ---------------------------------------------------------------------------
// ---------------------------------------------------- AcquireCriticalSection
inline AutoEnterCriticalSection EnterAutoCriticalSection(AutoCriticalSection& handle)
{
	EnterCriticalSection(handle);
	return AutoEnterCriticalSection(handle);
}
