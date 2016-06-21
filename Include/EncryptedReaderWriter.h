// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#pragma once

#include "BidirectionalPipe.h"
#include "WinCryptography.h"
#include "KeyExchange.h"

class EncryptedReaderWriter : public IReaderWriter
{
	IReaderWriter& readerWriter;
	Crypto crypto;

public:
	EncryptedReaderWriter(IReaderWriter& readerWriter, bool initiator)
		: readerWriter(readerWriter)
		, crypto(KeyExchange(readerWriter, initiator))
	{
	}

private:
	DWORD readerWriter_Write(const BYTE* buffer, DWORD length)
	{
		auto encryptedBuffer = this->crypto.Encrypt(std::vector<BYTE>(buffer, buffer + length), false);
		return this->readerWriter.Write(&encryptedBuffer[0], encryptedBuffer.size());
	}

	std::vector<unsigned char> readerWriter_Read(DWORD length)
	{
		return this->crypto.Decrypt(this->readerWriter.Read(length), false);
	}
};
