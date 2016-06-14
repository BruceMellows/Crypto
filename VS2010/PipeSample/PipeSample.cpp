// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#include "stdafx.h"

#include "tstring.h"
#include "tstream.h"
#include "BidirectionalPipe.h"
#include "AutoClosableHandle.h"
#include "Crypto.h"
#include "EncryptedPipe.h"

static LPCTSTR sampleMessageText = TEXT("*** Hello World from the land of PIPE ***");

DWORD WINAPI RunClient(const std::tstring& pipeName)
{
    std::tcout << "CLIENT: Connecting to pipe " << pipeName << " ..." << std::endl;
	auto rawPipe = BidirectionalPipe::ClientPipe(pipeName);

#if 0
	// FIXME - share secret correctly
	auto cryptProvider = AcquireCryptProvider();
	auto sharedKey = GenRandom(cryptProvider, 32);
#else
	auto pipeNameIter = (BYTE*)&pipeName[0];
	std::vector<BYTE> sharedKey(pipeNameIter, pipeNameIter + (pipeName.length() * sizeof(pipeName[0])));
#endif
	EncryptedPipe pipe(rawPipe, sharedKey);

    std::tcout << "CLIENT: Reading data from pipe..." << std::endl;

    // The read operation will block until there is data to read
	auto buffer = pipe.Read(127 * sizeof(wchar_t));

	// convert the returned buffer to a wstring and report
	std::wstring text((wchar_t*)(&buffer[0]), buffer.size() / sizeof(wchar_t));
	std::wstring expected(sampleMessageText);
    std::tcout << "CLIENT: Number of bytes read: " << buffer.size() << std::endl;
    std::tcout << "CLIENT: Message: " << text.c_str() << std::endl;

	if (expected == text)
	{
		std::tcout << "CLIENT: Done." << std::endl;
	}
	else
	{
		std::tcout << "CLIENT: FAILED." << std::endl;
	}

	return 0;
}

DWORD WINAPI RunServer(const std::tstring& pipeName)
{
    std::tcout << "SERVER: Creating an instance of a named pipe (connected) " << pipeName.c_str() << " ..." << std::endl;
	auto rawPipe = BidirectionalPipe::ServerPipe(pipeName);

#if 0
	// FIXME - share secret correctly
	auto cryptProvider = AcquireCryptProvider();
	auto sharedKey = GenRandom(cryptProvider, 32);
#else
	auto pipeNameIter = (BYTE*)&pipeName[0];
	std::vector<BYTE> sharedKey(pipeNameIter, pipeNameIter + (pipeName.length() * sizeof(pipeName[0])));
#endif
	EncryptedPipe pipe(rawPipe, sharedKey);

    std::tcout << "SERVER: Sending data to pipe..." << std::endl;
    std::tstring data(sampleMessageText);
    DWORD numBytesWritten = pipe.Write((BYTE*)&data[0], data.length() * sizeof(data[0]));

	std::tcout << "SERVER: Number of bytes sent: " << numBytesWritten << std::endl;
    std::tcout << "SERVER: Done." << std::endl;

	return 0;
}

DWORD WINAPI PipeClientThreadProc(LPVOID lpParameter)
{
	return RunClient((const TCHAR*)lpParameter);
}

DWORD WINAPI PipeServerThreadProc(LPVOID lpParameter)
{
	Sleep(1000);
	return RunServer((const TCHAR*)lpParameter);
}

void WaitForThread(DWORD threadId, DWORD milliseconds)
{
	AutoClosableHandle thread(OpenThread(THREAD_ALL_ACCESS, FALSE, threadId));
	if (WAIT_OBJECT_0 != WaitForSingleObject(thread, milliseconds))
	{
		TerminateThread(thread, 0);
	}
}

#define THREAD_WAIT_SECONDS (5)
int _tmain(int argc, _TCHAR* argv[])
{
	if (argc > 1)
	{
		std::tstring arg1(argv[1]);
		if (arg1.length() > 7)
		{
			std::tstring pipeName(arg1.substr(7)); // MAYBE, MAYBE NOT

			if (arg1.substr(0, 7) == TEXT("thread:"))
			{
				DWORD clientThreadId;
				DWORD serverThreadId;
				AutoClosableHandle clientThread(CreateThread(NULL, 0, PipeClientThreadProc, LPVOID(pipeName.c_str()), 0, &clientThreadId));
				AutoClosableHandle ser4verThread(CreateThread(NULL, 0, PipeServerThreadProc, LPVOID(pipeName.c_str()), 0, &serverThreadId));
				WaitForThread(clientThreadId, THREAD_WAIT_SECONDS * 1000);
				WaitForThread(serverThreadId, THREAD_WAIT_SECONDS * 1000);
			}
			else if (arg1.substr(0, 7) == TEXT("server:"))
			{
				return RunServer(pipeName);
			}
			else if (arg1.substr(0, 7) == TEXT("client:"))
			{
				return RunClient(pipeName);
			}
		}
	}
	return 0;
}