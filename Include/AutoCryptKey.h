// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#pragma once

#include "AutoCryptProvider.h"
#include "AutoCryptHash.h"

#include <vector>

//////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////// CryptKeyHandle
struct CryptKeyHandle
{
	typedef HCRYPTKEY handle_type;
	static	handle_type	DefaultHandle() { return 0; }
	static	void		DestroyHandle(handle_type handle) { CryptDestroyKey(handle); }
};
typedef AutoHandle<CryptKeyHandle> AutoCryptKey;

// ---------------------------------------------------------------------------
// ----------------------------------------------------------- AcquireCryptKey
inline AutoCryptKey AcquireCryptKey(
	const AutoCryptProvider& cryptProvider,
	const AutoCryptHash& cryptHash,
	const std::vector<BYTE>& sharedKey,
	ALG_ID keyAlg,
	int keyLength)
{
	if (!CryptHashData(cryptHash, &sharedKey[0], sharedKey.size(), 0))
	{
		throw TEXT("CryptHashData(*cryptHash, &buffer[0], buffer.size(), 0)");
	}

	HCRYPTKEY hKey;
	if (!CryptDeriveKey(cryptProvider, keyAlg, cryptHash, keyLength, &hKey))
	{
		throw TEXT("CryptDeriveKey(*cryptProvider, keyAlg, *cryptHash, keyLength, &hKey)");
	}

	return AutoCryptKey(hKey);
}

// ---------------------------------------------------------------------------
// ------------------------------------------===-------------- AcquireCryptKey
inline AutoCryptKey AcquireCryptKey(
	const AutoCryptProvider& cryptProvider,
	const AutoCryptHash& cryptHash,
	const std::string& text,
	ALG_ID keyAlg,
	int keyLength)
{
	return AcquireCryptKey(
		cryptProvider,
		cryptHash,
		std::vector<BYTE>(text.begin(), text.end()),
		keyAlg,
		keyLength);
}
