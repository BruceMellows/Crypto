// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#pragma once

#include "AutoHandle.h"
#include <vector>

#include <Windows.h>

//////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// CryptProviderHandle
struct CryptProviderHandle
{
	typedef HCRYPTPROV handle_type;
	static	handle_type	DefaultHandle() { return 0; }
	static	void		DestroyHandle(handle_type handle) { CryptReleaseContext(handle, 0); }
};
typedef AutoHandle<CryptProviderHandle> AutoCryptProvider;

// ---------------------------------------------------------------------------
// ------------------------------------------------------ AcquireCryptProvider
inline AutoCryptProvider AcquireCryptProvider()
{
	HCRYPTPROV handle;

	if (!CryptAcquireContext(&handle, NULL, NULL, PROV_RSA_FULL, 0))
	{
		throw TEXT("CryptAcquireContext(&handle, NULL, NULL, PROV_RSA_FULL, 0)");
	}

	return AutoCryptProvider(handle);
}

// ---------------------------------------------------------------------------
// -------------------------------------------------------------------- Random
inline std::vector<unsigned char> GenRandom(const AutoCryptProvider& cryptProvider, DWORD length)
{
	std::vector<BYTE> result(length);
	if (!CryptGenRandom(cryptProvider, length, &result[0]))
	{
		throw TEXT("std::vector<unsigned char> GenRandom(AutoCryptProvider& cryptProvider, DWORD length) ... CryptGenRandom");
	}

	return result;
}
