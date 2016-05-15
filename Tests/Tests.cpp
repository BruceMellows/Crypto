// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#include "stdafx.h"

#include "Crypto.h"

static void TestGeneratePassword(const Crypto& crypto, const std::string dictionary, unsigned int len)
{
	auto password = crypto.GeneratePassword(dictionary, len);

	if (password.length() != len)
	{
		throw TEXT("Crypto::GeneratePassword length error");
	}

	for (auto iter = password.begin() ; iter != password.end() ; ++iter)
	{
		if (dictionary.find(*iter) == std::string::npos)
		{
			throw TEXT("Crypto::GeneratePassword dictionary error");
		}
	}
}

static void TestEncryptDecrypt(const Crypto& crypto)
{
	std::string content("content");
	auto encrypted = crypto.Encrypt(std::vector<BYTE>(content.begin(), content.end()), true);
	auto decrypted = crypto.Decrypt(encrypted, true);

	for (auto index = 0 ; index != content.length() ; ++index)
	{
		if (content[index] == encrypted[index])
		{
			throw TEXT("Crypto::Encrypt encrypted content error");
		}

		if (content[index] != decrypted[index])
		{
			throw TEXT("Crypto::Encrypt decrypted content error");
		}
	}
}

void RunTests()
{
	std::string entropy("entropy");
	Crypto crypto(std::vector<BYTE>(entropy.begin(), entropy.end()));

	TestGeneratePassword(crypto, "0123456789abcdef", 16);

	TestEncryptDecrypt(crypto);
}