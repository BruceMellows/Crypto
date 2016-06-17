// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#pragma once

#include <Windows.h>
#include <vector>

class IReaderWriter
{
public:
	DWORD Write(const BYTE* buffer, DWORD length) { return this->readerWriter_Write(buffer, length); }
	std::vector<unsigned char> Read(DWORD length) { return this->readerWriter_Read(length); }

private:
	virtual DWORD readerWriter_Write(const BYTE* buffer, DWORD length) =0;
	virtual std::vector<unsigned char> readerWriter_Read(DWORD length) =0;
};
