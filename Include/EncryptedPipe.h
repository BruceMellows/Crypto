// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#pragma once

#include "BidirectionalPipe.h"
#include "Crypto.h"

class EncryptedPipe
{
	BidirectionalPipe& pipe;
	Crypto crypto;

public:
	EncryptedPipe(BidirectionalPipe& pipe, std::vector<BYTE> sharedKey)
		: pipe(pipe)
		, crypto(sharedKey)
	{
	}

	DWORD Write(const BYTE* buffer, DWORD length)
	{
		auto encryptedBuffer = this->crypto.Encrypt(std::vector<BYTE>(buffer, buffer + length), false);
		return this->pipe.Write(&encryptedBuffer[0], encryptedBuffer.size());
	}

	std::vector<unsigned char> Read(DWORD length)
	{
		return this->crypto.Decrypt(this->pipe.Read(length), false);
	}
};
