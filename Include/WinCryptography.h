// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#pragma once

#include "AutoCryptProvider.h"
#include "IRandomSource.h"

class WinCryptography : public IRandomSource
{
	AutoCryptProvider cryptProvider;

public:
	WinCryptography()
		: cryptProvider(AcquireCryptProvider())
	{
	}

private:
	virtual void randomsource_GetBytes(unsigned char* dest, unsigned int count) const
	{
		auto buffer = GenRandom(this->cryptProvider, count);
		memcpy(dest, &buffer[0], buffer.size());
	}
};
