// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#pragma once

#include <Windows.h>

#include "AutoHandle.h"
#include "AutoLastError.h"

//////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////// NamedPipeHandle
struct NamedPipeHandle
{
	typedef HANDLE handle_type;
	static	handle_type	DefaultHandle() { return INVALID_HANDLE_VALUE; }
	static	void		DestroyHandle(handle_type handle) { CloseHandle(handle); }
};
typedef AutoHandle<NamedPipeHandle> AutoNamedPipe;

// ---------------------------------------------------------------------------
// -------------------------------------------------------------------- Create
inline AutoNamedPipe CreateNamedPipe(LPCTSTR lpName)
{
	auto handle = CreateNamedPipe(
		lpName,
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_BYTE,
		1,
		0,
		0,
		0,
		NULL);

	return AutoNamedPipe(handle);
}

inline AutoNamedPipe WaitForNamedPipe(LPCTSTR lpName)
{
	while (WaitNamedPipe(lpName, NMPWAIT_USE_DEFAULT_WAIT) == 0)
	{
		if ((DWORD)AutoLastError() == ERROR_FILE_NOT_FOUND)
		{
			Sleep(1);
			continue;
		}

		return AutoNamedPipe();
	}

	// Open the named pipe
	// Most of these parameters aren't very relevant for pipes.
	return AutoNamedPipe(
		CreateFile(
			lpName,
			GENERIC_READ, // only need read access
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
	));
}