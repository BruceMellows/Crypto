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
#include "EllipticCurve25519.h"
#include "Crypto.h"
#include "EncryptedPipe.h"
#include "WinCryptography.h"
#include "AutoEnterCriticalSection.h"

static LPCTSTR sampleMessageText = TEXT("*** Hello World from the land of PIPE ***");

struct ThreadParams
{
	std::tstring pipeName;
	AutoCriticalSection& criticalSection;

	ThreadParams(const std::tstring& pipeName, AutoCriticalSection& criticalSection)
		: pipeName(pipeName)
		, criticalSection(criticalSection)
	{
	}
};

DWORD WINAPI RunClient(ThreadParams* threadParams)
{
	{
		auto ecs = EnterAutoCriticalSection(threadParams->criticalSection);
		std::tcout << "CLIENT: Connecting to pipe " << threadParams->pipeName << " ..." << std::endl;
	}

	auto rawPipe = BidirectionalPipe::ClientPipe(threadParams->pipeName);

	WinCryptography randomSource;
	EllipticCurve25519::Keys clientKeys(randomSource);

	{
		auto ecs = EnterAutoCriticalSection(threadParams->criticalSection);
		std::tcout << "CLIENT: Private Key: " << clientKeys.GetPrivateKey().ToWString() << std::endl;
		std::tcout << "CLIENT: Public Key: " << clientKeys.GetPublicKey().ToWString() << std::endl;
	}

	auto clientKey = L"EC25519(" + clientKeys.GetPublicKey().ToWString() +L")";
	auto clientKeyIter = (BYTE*)&clientKey[0];
	auto clientKeyLength = clientKey.length() * sizeof(clientKey[0]);
	auto serverKeyBuffer = rawPipe.Read(clientKeyLength);
	std::wstring serverKey((wchar_t*)(&serverKeyBuffer[0]), serverKeyBuffer.size() / sizeof(wchar_t));
	auto serverPublicKey = EllipticCurve25519::PublicKey::FromWString(serverKey.substr(8, serverKey.length() - 9));
	rawPipe.Write(clientKeyIter, clientKeyLength);
	auto sharedKey = clientKeys.CreateSharedKey(serverPublicKey);

	{
		auto ecs = EnterAutoCriticalSection(threadParams->criticalSection);
		std::tcout << "CLIENT: Shared Key: " << sharedKey.ToWString() << std::endl;
	}

	EncryptedPipe pipe(rawPipe, sharedKey.ToBinary());

	{
		auto ecs = EnterAutoCriticalSection(threadParams->criticalSection);
		std::tcout << "CLIENT: Reading data from pipe..." << std::endl;
	}

	// The read operation will block until there is data to read
	auto buffer = pipe.Read(127 * sizeof(wchar_t));

	// convert the returned buffer to a wstring and report
	std::wstring text((wchar_t*)(&buffer[0]), buffer.size() / sizeof(wchar_t));
	std::wstring expected(sampleMessageText);

	{
		auto ecs = EnterAutoCriticalSection(threadParams->criticalSection);
		std::tcout << "CLIENT: Number of bytes read: " << buffer.size() << std::endl;
		std::tcout << "CLIENT: Message: " << text.c_str() << std::endl;
	}

	{
		auto ecs = EnterAutoCriticalSection(threadParams->criticalSection);
		if (expected == text)
		{
			std::tcout << "CLIENT: Done." << std::endl;
		}
		else
		{
			std::tcout << "CLIENT: FAILED." << std::endl;
		}
	}

	return 0;
}

DWORD WINAPI RunServer(ThreadParams* threadParams)
{
	{
		auto ecs = EnterAutoCriticalSection(threadParams->criticalSection);
		std::tcout << "SERVER: Creating an instance of a named pipe (connected) " << threadParams->pipeName << " ..." << std::endl;
	}

	auto rawPipe = BidirectionalPipe::ServerPipe(threadParams->pipeName);

	WinCryptography randomSource;
	EllipticCurve25519::Keys serverKeys(randomSource);

	{
		auto ecs = EnterAutoCriticalSection(threadParams->criticalSection);
		std::tcout << "SERVER: Private Key: " << serverKeys.GetPrivateKey().ToWString() << std::endl;
		std::tcout << "SERVER: Public Key: " << serverKeys.GetPublicKey().ToWString() << std::endl;
	}

	auto serverKey = L"EC25519(" + serverKeys.GetPublicKey().ToWString() +L")";
	auto serverKeyIter = (BYTE*)&serverKey[0];
	auto serverKeyLength = serverKey.length() * sizeof(serverKey[0]);
	rawPipe.Write(serverKeyIter, serverKeyLength);
	auto clientKeyBuffer = rawPipe.Read(serverKeyLength);
	std::wstring clientKey((wchar_t*)(&clientKeyBuffer[0]), clientKeyBuffer.size() / sizeof(wchar_t));
	auto clientPublicKey = EllipticCurve25519::PublicKey::FromWString(clientKey.substr(8, clientKey.length() - 9));
	auto sharedKey = serverKeys.CreateSharedKey(clientPublicKey);

	{
		auto ecs = EnterAutoCriticalSection(threadParams->criticalSection);
		std::tcout << "SERVER: Shared Key: " << sharedKey.ToWString() << std::endl;
	}

	EncryptedPipe pipe(rawPipe, sharedKey.ToBinary());

	{
		auto ecs = EnterAutoCriticalSection(threadParams->criticalSection);
		std::tcout << "SERVER: Sending data to pipe..." << std::endl;
	}

	std::tstring data(sampleMessageText);
	DWORD numBytesWritten = pipe.Write((BYTE*)&data[0], data.length() * sizeof(data[0]));

	{
		auto ecs = EnterAutoCriticalSection(threadParams->criticalSection);
		std::tcout << "SERVER: Number of bytes sent: " << numBytesWritten << std::endl;
		std::tcout << "SERVER: Done." << std::endl;
	}

	return 0;
}

DWORD WINAPI PipeClientThreadProc(LPVOID lpParameter)
{
	return RunClient((ThreadParams*)lpParameter);
}

DWORD WINAPI PipeServerThreadProc(LPVOID lpParameter)
{
	Sleep(1000);
	return RunServer((ThreadParams*)lpParameter);
}

void WaitForThread(DWORD threadId, DWORD milliseconds)
{
	AutoClosableHandle thread(OpenThread(THREAD_ALL_ACCESS, FALSE, threadId));
	if (WAIT_OBJECT_0 != WaitForSingleObject(thread, milliseconds))
	{
		TerminateThread(thread, 0);
	}
}

#define THREAD_WAIT_SECONDS (500)
int _tmain(int argc, _TCHAR* argv[])
{
	auto criticalSection = CreateAutoCriticalSection();
	if (argc > 1)
	{
		std::tstring arg1(argv[1]);

		// I made all the commands 7 characters long
		if (arg1.length() > 7)
		{
			std::tstring pipeCommand(arg1.substr(0, 7));
			std::tstring pipeParameter(arg1.substr(7));

			ThreadParams threadParams(pipeParameter, criticalSection);

			if (pipeCommand == TEXT("thread:"))
			{
				DWORD clientThreadId;
				AutoClosableHandle clientThread(CreateThread(NULL, 0, PipeClientThreadProc, LPVOID(&threadParams), 0, &clientThreadId));

				DWORD serverThreadId;
				AutoClosableHandle serverThread(CreateThread(NULL, 0, PipeServerThreadProc, LPVOID(&threadParams), 0, &serverThreadId));

				WaitForThread(clientThreadId, THREAD_WAIT_SECONDS * 1000);
				WaitForThread(serverThreadId, THREAD_WAIT_SECONDS * 1000);
			}
			else if (pipeCommand == TEXT("server:"))
			{
				return RunServer(&threadParams);
			}
			else if (pipeCommand == TEXT("client:"))
			{
				return RunClient(&threadParams);
			}
		}
	}
	return 0;
}