// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#pragma once

#include <Windows.h>
#include <vector>

#include "IReaderWriter.h"
#include "WinCryptography.h"
#include "EllipticCurve25519.h"

inline std::vector<unsigned char> KeyExchange(IReaderWriter& readerWriter, bool initiator)
{
	WinCryptography randomSource;
	EllipticCurve25519::Keys localKeys(randomSource);

	auto localPublicKeyText = L"EC25519(" + localKeys.GetPublicKey().ToWString() +L")";
	auto localPublicKeyIter = (BYTE*)&localPublicKeyText[0];
	auto localPublicKeyLength = localPublicKeyText.length() * sizeof(localPublicKeyText[0]);

	std::vector<unsigned char> remoteKeyBuffer;
	if (initiator)
	{
		readerWriter.Write(localPublicKeyIter, localPublicKeyLength);
		remoteKeyBuffer = readerWriter.Read(localPublicKeyLength);
	}
	else
	{
		remoteKeyBuffer = readerWriter.Read(localPublicKeyLength);
		readerWriter.Write(localPublicKeyIter, localPublicKeyLength);
	}

	std::wstring remotePublicKeyText((wchar_t*)(&remoteKeyBuffer[0]), remoteKeyBuffer.size() / sizeof(localPublicKeyText[0]));
	auto remotePublicKey = EllipticCurve25519::PublicKey::FromWString(remotePublicKeyText.substr(8, remotePublicKeyText.length() - 9));

	return localKeys.CreateSharedKey(remotePublicKey).ToBinary();
}
