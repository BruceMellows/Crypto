// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#pragma once

#include "tstring.h"
#include "AutoNamedPipe.h"
#include "IReaderWriter.h"
#include <vector>

class BidirectionalPipe : public IReaderWriter
{
	std::tstring pipeName;
	AutoNamedPipe pipeHandle;

private:
	BidirectionalPipe(const std::tstring& pipeName, NamedPipeHandle::handle_type pipeHandle)
		: pipeName(pipeName)
		, pipeHandle(pipeHandle)
	{
		if (!this->pipeHandle)
		{
			throw TEXT("BidirectionalPipe::BidirectionalPipe(std::wstring pipeName, HANDLE pipeHandle) !this->pipeHandle");
		}
	}

public:
	~BidirectionalPipe() {}

	BidirectionalPipe(BidirectionalPipe&& that)
		: pipeName(that.pipeName)
		, pipeHandle(that.pipeHandle.Yoink())
	{
	}

	BidirectionalPipe& Connect()
	{
		if (!ConnectNamedPipe(this->pipeHandle, NULL))
		{
			throw TEXT("BidirectionalPipe::Connect(std::wstring pipeName) !ConnectNamedPipe(this->pipeHandle, NULL)");
		}

		return *this;
	}

	static BidirectionalPipe ServerPipe(const std::tstring& pipeName)
	{
		auto fullPipeName = std::tstring(TEXT("\\\\.\\pipe\\")).append(pipeName);
		BidirectionalPipe result(
			pipeName,
			CreateNamedPipe(
				fullPipeName.c_str(),
				PIPE_ACCESS_DUPLEX,
				PIPE_TYPE_BYTE,
				1,
				0,
				0,
				0,
				NULL));
		result.Connect();
		return result;
	}

	static BidirectionalPipe ClientPipe(const std::tstring& pipeName)
	{
		auto fullPipeName = std::tstring(TEXT("\\\\.\\pipe\\")).append(pipeName);
		return BidirectionalPipe(pipeName, WaitForNamedPipe(fullPipeName.c_str()).Yoink());
	}

private:
	DWORD readerWriter_Write(const BYTE* buffer, DWORD length)
	{
		DWORD totalBytesWritten = 0;
		if (!WriteFile(this->pipeHandle, buffer, length, &totalBytesWritten, NULL))
		{
			throw TEXT("BidirectionalPipe::Write(const BYTE* buffer, DWORD length) !WriteFile(this->pipeHandle, begin, length, &totalBytesWritten, NULL)");
		}

		return totalBytesWritten;
	}

	std::vector<unsigned char> readerWriter_Read(DWORD length)
	{
		// The read operation will block until there is data to read
		std::vector<unsigned char> result(length, 0);
		DWORD numBytesRead = 0;
		if (!ReadFile(
			this->pipeHandle,
			&result[0],       // the data from the pipe will be put here
			length,           // number of bytes allocated
			&numBytesRead,    // this will store number of bytes actually read
			NULL))            // not using overlapped IO
		{
			throw TEXT("BidirectionalPipe::Read(DWORD length) !ReadFile(this->pipeHandle, &result[0], length, &numBytesRead, NULL)");
		}

		result.resize(numBytesRead);
		return result;
	}

private:
	BidirectionalPipe(const BidirectionalPipe&);
	BidirectionalPipe& operator = (const BidirectionalPipe&);
};
