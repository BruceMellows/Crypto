// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#pragma once

#include <io.h>
#include <fcntl.h>

#include "BidirectionalPipe.h"
#include "WinCryptography.h"
#include "KeyExchange.h"

class StdioReaderWriter : public IReaderWriter
{
public:
	StdioReaderWriter()
	{
		_setmode(_fileno(stdout), _O_BINARY);
		_setmode(_fileno(stdin), _O_BINARY);
	}

private:
	DWORD readerWriter_Write(const BYTE* buffer, DWORD length)
	{
		return _write(_fileno(stdout), buffer, length);
	}

	std::vector<unsigned char> readerWriter_Read(DWORD length)
	{
		std::vector<unsigned char> result(length, 0);
		result.resize(_read(_fileno(stdin), &result[0], length));
		return result;
	}
};
