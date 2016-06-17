// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#pragma once

#include <string>
#include <iostream>
#include <vector>

#include "IRandomSource.h"

namespace EllipticCurve25519 {
	class PublicKey {
		void*	pimpl;

	public:

		PublicKey();
		~PublicKey();
		PublicKey(PublicKey&&);

		std::wstring ToWString() const;
		static PublicKey FromWString(const std::wstring& publicKeyHex);

	private:
		PublicKey(const PublicKey&);
		PublicKey& operator = (const PublicKey&);
	};

	class SharedKey {
		void*	pimpl;

	public:

		SharedKey();
		~SharedKey();
		SharedKey(SharedKey&&);

		std::wstring ToWString() const;
		std::vector<unsigned char> ToBinary() const;

	private:
		SharedKey(const SharedKey&);
		SharedKey& operator = (const SharedKey&);
	};

	class PrivateKey {
		void*	pimpl;

	public:

		PrivateKey();
		~PrivateKey();
		PrivateKey(PrivateKey&&);

		std::wstring ToWString() const;

	private:
		PrivateKey(const PrivateKey&);
		PrivateKey& operator = (const PrivateKey&);
	};

	class Keys {
		PublicKey	publicKey;
		PrivateKey	privateKey;

	public:

		Keys(const IRandomSource&);

		inline const PublicKey&	GetPublicKey() const { return this->publicKey; }
		inline const PrivateKey&	GetPrivateKey() const { return this->privateKey; }

		SharedKey CreateSharedKey(const PublicKey& publicKey) const;
	};
}

inline std::wostream& operator <<(std::wostream& stream, const EllipticCurve25519::PrivateKey& key) {
	return stream << key.ToWString().c_str();
}

inline std::wostream& operator <<(std::wostream& stream, const EllipticCurve25519::PublicKey& key) {
	return stream << key.ToWString().c_str();
}

inline std::wostream& operator <<(std::wostream& stream, const EllipticCurve25519::SharedKey& key) {
	return stream << key.ToWString().c_str();
}
