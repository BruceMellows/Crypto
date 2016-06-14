// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#pragma once

#include "AutoCryptProvider.h"
#include "AutoCryptHash.h"
#include "AutoCryptKey.h"
#include "Cryptography.h"

#include <vector>

//////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////// Crypto
class Crypto : public Cryptography::RandomSource
{
private:

	AutoCryptProvider	cryptProvider;
	AutoCryptHash		cryptHash;
	AutoCryptKey		cryptKey;

public:

	inline Crypto::Crypto(const std::vector<BYTE>& sharedKey)
		: cryptProvider(AcquireCryptProvider())
		, cryptHash(AcquireCryptHash(cryptProvider, CALG_MD5))
		, cryptKey(AcquireCryptKey(cryptProvider, cryptHash, sharedKey, CALG_RC4, 0x00800000))
	{
	}

	//////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////// Password
	template<class TSTRING> inline TSTRING GeneratePassword(const TSTRING& dictionary, const unsigned int len) const
	{
		std::vector<TSTRING::value_type> buffer;
		buffer.reserve(len);
		while (buffer.size() < len)
		{
			auto bytes = GenRandom(this->cryptProvider, len - buffer.size());
			for (auto iter = bytes.begin() ; iter != bytes.end() ; ++iter)
			{
				if (*iter < dictionary.length())
				{
					buffer.push_back(dictionary[*iter]);
				}
			}
		}

		return TSTRING(buffer.begin(), buffer.end());
	}

	//////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////// Encryption
	inline std::vector<BYTE> Encrypt(const std::vector<BYTE>& bytes, bool endOfStream) const
	{
		DWORD encryptedSize = bytes.size();
		std::vector<BYTE> buffer(encryptedSize, 0);
		memcpy_s(&buffer[0], encryptedSize, (const void*)&bytes[0], bytes.size());
		if(!CryptEncrypt(this->cryptKey, NULL, endOfStream, 0, &buffer[0], &encryptedSize, encryptedSize))
		{
			throw TEXT("std::vector<BYTE> Encrypt(const std::vector<BYTE>& bytes) const ... CryptEncrypt");
		}

		return buffer;
	}

	inline std::vector<BYTE> Decrypt(const std::vector<BYTE>& bytes, bool endOfStream) const
	{
		DWORD decryptedSize = bytes.size();
		std::vector<BYTE> buffer(decryptedSize, 0);
		memcpy_s(&buffer[0], decryptedSize, (const void*)&bytes[0], bytes.size());
		if(!CryptDecrypt(this->cryptKey, NULL, endOfStream, 0, &buffer[0], &decryptedSize))
		{
			throw TEXT("std::vector<BYTE> Decrypt(const std::vector<BYTE>& bytes) const ... CryptDecrypt");
		}

		return buffer;
	}

private:
	/*override*/ void randomsource_GetBytes(unsigned char* dest, unsigned int count) const
	{
		auto buffer = GenRandom(this->cryptProvider, count);
		std::copy(buffer.begin(), buffer.end(), dest);
	}
};
