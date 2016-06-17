// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#include "stdafx.h"

#include "EllipticCurve25519.h"
#include "curve25519_i64.h"
#include <algorithm>

#define rangeof(x) (sizeof(x)/sizeof(*(x)))

namespace EllipticCurve25519 {
	const wchar_t hexdigits[] = L"0123456789ABCDEF";

	inline std::wstring ToWString(const void* pv, int n)
	{
		const unsigned char* p = (const unsigned char*)pv;
		std::wstring result;
		for(auto i = 0 ; i != n ; ++i)
		{
			auto byte = *p++;
			result += hexdigits[byte >> 4];
			result += hexdigits[byte & 0xf];
		}

		return result;
	}

	inline unsigned char FromWChar(const wchar_t hiHex, const wchar_t loHex)
	{
		unsigned char lo = std::distance(hexdigits, std::find(hexdigits, hexdigits + rangeof(hexdigits), loHex));
		unsigned char hi = std::distance(hexdigits, std::find(hexdigits, hexdigits + rangeof(hexdigits), hiHex));
		return lo + (hi << 4);
	}

	inline void FromWString(const std::wstring text, unsigned char* outputIter, int length)
	{
		for (auto inputIter = text.begin() ; length && inputIter != text.end() ; --length)
		{
			if (inputIter != text.end())
			{
				wchar_t hi = *inputIter++;
				if (inputIter != text.end())
				{
					wchar_t lo = *inputIter++;
					*outputIter = FromWChar(hi, lo);
					++outputIter;
				}
			}
		}
	}

	PublicKey::PublicKey()
		: pimpl(new k25519) {
	}

	PublicKey::~PublicKey() {
		if (this->pimpl) {
			delete this->pimpl;
		}
	}

	PublicKey::PublicKey(PublicKey&& that)
		: pimpl(that.pimpl) {
			that.pimpl = 0;
	}

	std::wstring PublicKey::ToWString() const {
		return EllipticCurve25519::ToWString(this->pimpl, sizeof(k25519));
	}

	PublicKey PublicKey::FromWString(const std::wstring& publicKeyHex) {
		PublicKey result;
		EllipticCurve25519::FromWString(publicKeyHex, (unsigned char*)result.pimpl, sizeof(k25519));
		return result;
	}

	SharedKey::SharedKey()
		: pimpl(new k25519) {
	}

	SharedKey::~SharedKey() {
		if (this->pimpl) {
			delete this->pimpl;
		}
	}

	SharedKey::SharedKey(SharedKey&& that)
		: pimpl(that.pimpl) {
			that.pimpl = 0;
	}

	std::wstring SharedKey::ToWString() const {
		return EllipticCurve25519::ToWString(this->pimpl, sizeof(k25519));
	}

	std::vector<unsigned char> SharedKey::ToBinary() const {
		auto begin = (unsigned char*)this->pimpl;
		return std::vector<unsigned char>(begin, begin + sizeof(k25519));
	}

	PrivateKey::PrivateKey()
		: pimpl(new k25519) {
	}

	PrivateKey::~PrivateKey() {
		if (this->pimpl) {
			delete this->pimpl;
		}
	}

	PrivateKey::PrivateKey(PrivateKey&& that)
		: pimpl(that.pimpl) {
			that.pimpl = 0;
	}

	std::wstring PrivateKey::ToWString() const {
		return EllipticCurve25519::ToWString(this->pimpl, sizeof(k25519));
	}

	Keys::Keys(const IRandomSource& entropySource) {
		entropySource.GetBytes(*(unsigned char**)(&this->privateKey), sizeof(k25519));
		::keygen25519(*(unsigned char**)(&this->publicKey), 0, *(unsigned char**)(&this->privateKey));
	}

	SharedKey Keys::CreateSharedKey(const PublicKey& publicKey) const {
		SharedKey result;
		::curve25519(*(unsigned char**)(&result), *(unsigned char**)(&this->privateKey), *(unsigned char**)(&publicKey));
		return result;
	}
}