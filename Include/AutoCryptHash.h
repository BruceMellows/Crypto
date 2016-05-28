// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#pragma once

#include "AutoCryptProvider.h"

//////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////// CryptHashHandle
struct CryptHashHandle
{
	typedef HCRYPTHASH handle_type;
	static	handle_type	DefaultHandle() { return 0; }
	static	void		DestroyHandle(handle_type handle) { CryptDestroyHash(handle); }
};
typedef AutoHandle<CryptHashHandle> AutoCryptHash;

// ---------------------------------------------------------------------------
// ---------------------------------------------------------- AcquireCryptHash
inline AutoCryptHash AcquireCryptHash(
	const AutoCryptProvider& cryptProvider,
	ALG_ID hashAlg)
{
	HCRYPTHASH handle;

	if (!CryptCreateHash(cryptProvider, hashAlg, 0, 0, &handle))
	{
		throw TEXT("CryptCreateHash(*cryptProvider, hashAlg, 0, 0, &handle)");
	}

	return AutoCryptHash(handle);
}
