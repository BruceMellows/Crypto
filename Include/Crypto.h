// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#pragma once

#include "AutoCryptProvider.h"
#include "AutoCryptHash.h"
#include "AutoCryptKey.h"

#include <vector>

//////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////// Crypto
class Crypto
{
private:

	AutoCryptProvider	cryptProvider;
	AutoCryptHash		cryptHash;
	AutoCryptKey		cryptKey;

public:

	inline Crypto::Crypto(const std::vector<BYTE>& entropy)
		: cryptProvider(AcquireCryptProvider())
		, cryptHash(AcquireCryptHash(cryptProvider, CALG_MD5))
		, cryptKey(AcquireCryptKey(cryptProvider, cryptHash, entropy, CALG_RC4, 0x00800000))
	{
	}

	//////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////// Random
	inline std::vector<BYTE> GenRandom(DWORD length) const
	{
		std::vector<BYTE> result(length);
		if (!CryptGenRandom(this->cryptProvider, length, &result[0]))
		{
			throw TEXT("std::vector<BYTE> Crypto::GenRandom(DWORD length) const ... CryptGenRandom");
		}

		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////// Password
	template<class TSTRING> inline TSTRING GeneratePassword(const TSTRING& dictionary, const unsigned int len) const
	{
		std::vector<TSTRING::value_type> buffer;
		buffer.reserve(len);
		while (buffer.size() < len)
		{
			auto bytes = GenRandom(len - buffer.size());
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
	inline std::vector<BYTE> Encrypt(const std::vector<BYTE>& bytes) const
	{
		DWORD encryptedSize = bytes.size();
		ArrayPtr<BYTE> buffer(new BYTE[encryptedSize]);
		memcpy_s((BYTE*)buffer, encryptedSize, (const void*)&bytes[0], bytes.size());
		if(!CryptEncrypt(this->cryptKey, NULL, TRUE, 0, buffer, &encryptedSize, encryptedSize))
		{
			throw TEXT("std::vector<BYTE> Encrypt(const std::vector<BYTE>& bytes) const ... CryptEncrypt");
		}

		return std::vector<BYTE>((BYTE*)buffer, &buffer[encryptedSize]);
	}

	inline std::vector<BYTE> Decrypt(const std::vector<BYTE>& bytes) const
	{
		DWORD decryptedSize = bytes.size();
		ArrayPtr<BYTE> buffer(new BYTE[decryptedSize]);
		memcpy_s((BYTE*)buffer, decryptedSize, (const void*)&bytes[0], bytes.size());
		if(!CryptDecrypt(this->cryptKey, NULL, TRUE, 0, buffer, &decryptedSize))
		{
			throw TEXT("std::vector<BYTE> Decrypt(const std::vector<BYTE>& bytes) const ... CryptDecrypt");
		}

		return std::vector<BYTE>((BYTE*)buffer, &buffer[decryptedSize]);
	}
};
