// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#include "stdafx.h"

#include "Crypto.h"
#include "EllipticCurve25519.h"
#include "Cryptography.h"

static int TestGeneratePassword(const Crypto& crypto, const std::string dictionary, unsigned int len)
{
	auto password = crypto.GeneratePassword(dictionary, len);

	if (password.length() != len)
	{
		std::wcout << L"Crypto::GeneratePassword length error" << std::endl;
		return 1;
	}

	for (auto iter = password.begin() ; iter != password.end() ; ++iter)
	{
		if (dictionary.find(*iter) == std::string::npos)
		{
			std::wcout << L"Crypto::GeneratePassword dictionary error" << std::endl;
			return 1;
		}
	}

	return 0;
}

static int TestEncryptDecrypt(const Crypto& cryptoA, const Crypto& cryptoB)
{
	std::string content= cryptoA.GeneratePassword(std::string("0123456789abcdefghijklmnopqrstuvABCDEFGHIJKLMNOPQRSTUVWXYZ"), 256000);
	auto encrypted = cryptoA.Encrypt(std::vector<BYTE>(content.begin(), content.end()), true);
	auto decrypted = cryptoB.Decrypt(encrypted, true);
	auto sameCount = 0;

	for (auto index = 0 ; index != content.length() ; ++index)
	{
		sameCount += content[index] == encrypted[index] ? 1 : 0;

		if (content[index] != decrypted[index])
		{
			std::wcout << L"Crypto::Encrypt decrypted content error" << std::endl;
			return 1;
		}
	}

	if (sameCount == content.length())
	{
		std::wcout << L"Crypto::Encrypt encrypted content error" << std::endl;
		return 1;
	}

	return 0;
}

class DudRandomSource : public Cryptography::RandomSource {
	unsigned char value;
public:
	DudRandomSource(unsigned char value) : value(value) { }
	void randomsource_GetBytes(unsigned char* dest, unsigned int count) const {
		memset(dest, this->value, count);
	}
};

static int TestCreateKey25519(const Cryptography::RandomSource& randomSource, const std::wstring& expected)
{
	auto keys = EllipticCurve25519::Keys(randomSource);

	auto actual = keys.GetPublicKey().ToWString();

	if (expected != actual)
	{
		std::wcout << L"EllipticCurve25519 Create public key content error" << std::endl;
		return 1;
	}

	return 0;
}

static int TestSharedKey25519(
	const Cryptography::RandomSource& randomSourceA,
	const Cryptography::RandomSource& randomSourceB,
	const std::wstring& expected)
{
	auto keysA = EllipticCurve25519::Keys(randomSourceA);
	auto keysB = EllipticCurve25519::Keys(randomSourceB);

	auto actual = keysA.CreateSharedKey(keysB).ToWString();

	if (expected != actual)
	{
		std::wcout << L"EllipticCurve25519 Create shared key content error" << std::endl;
		return 1;
	}

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::string entropy("entropy");
	Crypto cryptoA(std::vector<BYTE>(entropy.begin(), entropy.end()));
	Crypto cryptoB(std::vector<BYTE>(entropy.begin(), entropy.end()));

	int total = 5;
	int failed = 0
		+ TestGeneratePassword(cryptoA, "0123456789abcdef", 16)
		+ TestEncryptDecrypt(cryptoA, cryptoB)
		+ TestCreateKey25519(DudRandomSource(0x55), L"83BA66B48DF6777D6EB6DDA90E9792319AF48D3BA3210620E7B4641C4F88C476")
		+ TestCreateKey25519(DudRandomSource(0xAA), L"41ACE9D483B7CC3F75640E04D7AACA6C2BA8F44854FEA5158598D49B382E0407")
		+ TestSharedKey25519(DudRandomSource(0x55), DudRandomSource(0xAA), L"99982C6AA51244F9CF49295A8EF0B882E2FED6C131F106556C803143758946E2")
		;

	if (failed)
	{
		std::wcout << L"FAILED (" << failed << L" of " << total << L" tests)" << std::endl;
	}
	else
	{
		std::wcout << L"PASSED (" << total << L" tests)" << std::endl;
	}

	return 0;
}