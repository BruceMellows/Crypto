// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#include "stdafx.h"

#include "EllipticCurve25519.h"
#include "curve25519_i64.h"

namespace EllipticCurve25519 {
	const wchar_t hexdigits[] = L"0123456789ABCDEF";

	inline std::wstring ToWString(const void* pv, int n)
	{
		const unsigned char* p = (const unsigned char*)pv;
		std::wstring result;
		for(auto i = 0 ; i != n ; ++i)
		{
			auto byte = *p++;
			result += hexdigits[byte & 0xf];
			result += hexdigits[byte >> 4];
		}

		return result;
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
		return EllipticCurve25519::ToWString(this->pimpl, 32);
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
		return EllipticCurve25519::ToWString(this->pimpl, 32);
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
		return EllipticCurve25519::ToWString(this->pimpl, 32);
	}

	Keys::Keys(const Cryptography::RandomSource& entropySource) {
		entropySource.GetBytes(*(unsigned char**)(&this->privateKey), 32);
		::keygen25519(*(unsigned char**)(&this->publicKey), 0, *(unsigned char**)(&this->privateKey));
	}

	SharedKey Keys::CreateSharedKey(const Keys& that) const {
		SharedKey result;
		::curve25519(*(unsigned char**)(&result), *(unsigned char**)(&this->privateKey), *(unsigned char**)(&that.publicKey));
		return result;
	}
}