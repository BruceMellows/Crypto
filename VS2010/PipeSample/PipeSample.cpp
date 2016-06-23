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
#include "EncryptedReaderWriter.h"
#include "StdioReaderWriter.h"
#include "WinCryptography.h"
#include "AutoEnterCriticalSection.h"
#include "rangeof.h"

static LPCTSTR sampleMessageText = TEXT("*** Hello World from the land of PIPE ***");

struct ThreadParamsPipeName
{
	std::tstring pipeName;
	AutoCriticalSection& criticalSection;

	ThreadParamsPipeName(const std::tstring& pipeName, AutoCriticalSection& criticalSection)
		: pipeName(pipeName)
		, criticalSection(criticalSection)
	{
	}
};

struct ThreadParamsReaderWriter
{
	IReaderWriter& readerWriter;
	AutoCriticalSection& criticalSection;

	ThreadParamsReaderWriter(IReaderWriter& readerWriter, AutoCriticalSection& criticalSection)
		: readerWriter(readerWriter)
		, criticalSection(criticalSection)
	{
	}
};

class ChildProcess : public IReaderWriter
{
public:
	ChildProcess(const std::tstring& image)
	{
		this->Create(const_cast<LPTSTR>((TEXT("\"") + image + TEXT("\"")).c_str()));
	}

	ChildProcess(const std::tstring& image, const std::vector<std::tstring>& args)
	{
		std::tstring arg = TEXT("\"") + image + TEXT("\"");
		for (auto iter = args.begin() ; iter != args.end() ; ++iter)
		{
			arg += TEXT(" \"");
			arg += *iter;
			arg += TEXT("\"");
		}

		this->Create(const_cast<LPTSTR>(arg.c_str()));
	}

	void Wait(DWORD dwMilliseconds)
	{
		WaitForSingleObject(this->child_wait_thread, dwMilliseconds);
	}

private:
	HANDLE		parent_read_end;
	HANDLE		parent_write_end;
	HANDLE		child_read_end;
	HANDLE		child_write_end;
	HANDLE		child_wait_thread;

	static DWORD WINAPI static_TreadedWaitProc(LPVOID lpParameter)
	{
		WaitForSingleObject(lpParameter, INFINITE);
		return 0;
	}

	void Create(LPTSTR args)
	{
		SECURITY_ATTRIBUTES saAttr;
		HANDLE temp_parent_read_end;
		HANDLE temp_parent_write_end;
		HANDLE hSaveStdout;
		HANDLE hSaveStderr;
		HANDLE hSaveStdin;
		PROCESS_INFORMATION piProcInfo;
		STARTUPINFO siStartInfo;

		// Set the bInheritHandle flag so pipe handles are inherited.

		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;

		// The steps for redirecting child process's STDOUT:
		//     1. Save current STDOUT, to be restored later.
		//     2. Create anonymous pipe to be STDOUT for child process.
		//     3. Set STDOUT of the parent process to be write handle to
		//	  the pipe, so it is inherited by the child process.
		//     4. Create a noninheritable duplicate of the read handle and
		//	  close the inheritable read handle.

		// Save the handle to the current STDOUT.

		hSaveStdout = GetStdHandle(STD_OUTPUT_HANDLE);

		hSaveStderr = GetStdHandle(STD_ERROR_HANDLE);

		// Create a pipe for the child process's STDOUT.

		if (! CreatePipe(&temp_parent_read_end, &this->child_write_end, &saAttr, 0))
			throw std::tstring(TEXT("ChildProcess::Create(LPCTSTR image, LPTSTR args) ")) + TEXT("!CreatePipe(&temp_parent_read_end, &this->child_write_end, &saAttr, 0)");

		// Set a write handle to the pipe to be STDOUT.

		if (! SetStdHandle(STD_OUTPUT_HANDLE, this->child_write_end))
			throw std::tstring(TEXT("ChildProcess::Create(LPCTSTR image, LPTSTR args) ")) + TEXT("!SetStdHandle(STD_OUTPUT_HANDLE, this->child_write_end)");

		if (! SetStdHandle(STD_ERROR_HANDLE, this->child_write_end))
			throw std::tstring(TEXT("ChildProcess::Create(LPCTSTR image, LPTSTR args) ")) + TEXT("!SetStdHandle(STD_ERROR_HANDLE, this->child_write_end)");

		// Create noninheritable read handle and close the inheritable read
		// handle.

		if (! DuplicateHandle(GetCurrentProcess(), temp_parent_read_end,
					   GetCurrentProcess(), &this->parent_read_end , 0,
					   FALSE,
					   DUPLICATE_SAME_ACCESS))
			throw std::tstring(TEXT("ChildProcess::Create(LPCTSTR image, LPTSTR args) ")) + TEXT("!DuplicateHandle(GetCurrentProcess(), temp_parent_read_end, GetCurrentProcess(), &this->parent_read_end , 0, FALSE, DUPLICATE_SAME_ACCESS)");
		CloseHandle(temp_parent_read_end);

		// The steps for redirecting child process's STDIN:
		//     1.  Save current STDIN, to be restored later.
		//     2.  Create anonymous pipe to be STDIN for child process.
		//     3.  Set STDIN of the parent to be the read handle to the
		//	   pipe, so it is inherited by the child process.
		//     4.  Create a noninheritable duplicate of the write handle,
		//	   and close the inheritable write handle.

		// Save the handle to the current STDIN.

		hSaveStdin = GetStdHandle(STD_INPUT_HANDLE);

		// Create a pipe for the child process's STDIN.

		if (! CreatePipe(&this->child_read_end, &temp_parent_write_end, &saAttr, 0))
			throw std::tstring(TEXT("ChildProcess::Create(LPCTSTR image, LPTSTR args) ")) + TEXT("!CreatePipe(&this->child_read_end, &temp_parent_write_end, &saAttr, 0)");

		// Set a read handle to the pipe to be STDIN.

		if (! SetStdHandle(STD_INPUT_HANDLE, this->child_read_end))
			throw std::tstring(TEXT("ChildProcess::Create(LPCTSTR image, LPTSTR args) ")) + TEXT("!SetStdHandle(STD_INPUT_HANDLE, this->child_read_end)");

		// Duplicate the write handle to the pipe so it is not inherited.

		if (! DuplicateHandle(GetCurrentProcess(), temp_parent_write_end,
					   GetCurrentProcess(), &this->parent_write_end, 0,
					   FALSE,		   // not inherited
					   DUPLICATE_SAME_ACCESS))
			throw std::tstring(TEXT("ChildProcess::Create(LPCTSTR image, LPTSTR args) ")) + TEXT("!DuplicateHandle(GetCurrentProcess(), temp_parent_write_end, GetCurrentProcess(), &this->parent_write_end, 0, FALSE, DUPLICATE_SAME_ACCESS");

		CloseHandle(temp_parent_write_end);

		// Now create the child process.

		// Set up members of the PROCESS_INFORMATION structure.

		ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );

		// Set up members of the STARTUPINFO structure.

		ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
		siStartInfo.cb = sizeof(STARTUPINFO);

		// Create the child process. (SUSPENDED)

		if (!CreateProcess(NULL,
				   args,	  // command line
				   NULL,	  // process security attributes
				   NULL,	  // primary thread security attributes
				   TRUE,	  // handles are inherited
				   CREATE_SUSPENDED,		 // creation flags
				   NULL,	  // use parent's environment
				   NULL,	  // use parent's current directory
				   &siStartInfo,  // STARTUPINFO pointer
				   &piProcInfo))  // receives PROCESS_INFORMATION
			throw std::tstring(TEXT("ChildProcess::Create(LPCTSTR image, LPTSTR args) ")) + TEXT("!CreateProcess(NULL, args, NULL, NULL, TRUE, CREATE_SUSPENDED, NULL, NULL, &siStartInfo, &piProcInfo)");

		// create the waiter thread, then RESUME the child process

		DWORD threadID;
		this->child_wait_thread = CreateThread(0, 1, static_TreadedWaitProc, (LPVOID)piProcInfo.hProcess, 0, &threadID);
		ResumeThread(piProcInfo.hThread);

		// After process creation, restore the saved STDIN and STDOUT.

		if (! SetStdHandle(STD_INPUT_HANDLE, hSaveStdin))
			throw std::tstring(TEXT("ChildProcess::Create(LPCTSTR image, LPTSTR args) ")) + TEXT("!SetStdHandle(STD_INPUT_HANDLE, hSaveStdin)");

		if (! SetStdHandle(STD_ERROR_HANDLE, hSaveStderr))
			throw std::tstring(TEXT("ChildProcess::Create(LPCTSTR image, LPTSTR args) ")) + TEXT("!SetStdHandle(STD_ERROR_HANDLE, hSaveStderr)");

		if (! SetStdHandle(STD_OUTPUT_HANDLE, hSaveStdout))
			throw std::tstring(TEXT("ChildProcess::Create(LPCTSTR image, LPTSTR args) ")) + TEXT("!SetStdHandle(STD_OUTPUT_HANDLE, hSaveStdout)");
	}

	DWORD readerWriter_Write(const BYTE* buffer, DWORD length)
	{
		DWORD totalBytesWritten = 0;
		if (!WriteFile(this->parent_write_end, buffer, length, &totalBytesWritten, NULL))
		{
			throw TEXT("ChildProcess::readerWriter_Write(const BYTE* buffer, DWORD length) !WriteFile(this->parent_write_end, begin, length, &totalBytesWritten, NULL)");
		}

		return totalBytesWritten;
	}

	std::vector<unsigned char> readerWriter_Read(DWORD length)
	{
		// The read operation will block until there is data to read
		std::vector<unsigned char> result(length, 0);
		DWORD numBytesRead = 0;
		if (!ReadFile(
			this->parent_read_end,
			&result[0],       // the data from the pipe will be put here
			length,           // number of bytes allocated
			&numBytesRead,    // this will store number of bytes actually read
			NULL))            // not using overlapped IO
		{
			throw TEXT("ChildProcess::readerWriter_Read(DWORD length) !ReadFile(this->parent_read_end, &result[0], length, &numBytesRead, NULL)");
		}

		result.resize(numBytesRead);
		return result;
	}
};

DWORD WINAPI RunClient(IReaderWriter& readerWriter, AutoCriticalSection& criticalSection, bool silent =false)
{
	EncryptedReaderWriter pipe(readerWriter, false);

	if (!silent)
	{
		auto ecs = EnterAutoCriticalSection(criticalSection);
		std::tcout << "CLIENT: Reading data from pipe..." << std::endl;
	}

	// The read operation will block until there is data to read
	auto buffer = pipe.Read(127 * sizeof(wchar_t));

	// convert the returned buffer to a wstring and report
	std::wstring text((wchar_t*)(&buffer[0]), buffer.size() / sizeof(wchar_t));
	std::wstring expected(sampleMessageText);

	if (!silent)
	{
		auto ecs = EnterAutoCriticalSection(criticalSection);
		std::tcout << "CLIENT: Number of bytes read: " << buffer.size() << std::endl;
		std::tcout << "CLIENT: Message: " << text << std::endl;
	}

	if (silent)
	{
		if (expected != text)
		{
			MessageBox(NULL, TEXT("CLIENT FAILED"), TEXT("CLIENT FAILED"), MB_OK);
		}
	}
	else
	{
		if (expected == text)
		{
			auto ecs = EnterAutoCriticalSection(criticalSection);
			std::tcout << "CLIENT: Done." << std::endl;
		}
		else
		{
			auto ecs = EnterAutoCriticalSection(criticalSection);
			std::tcout << "CLIENT: FAILED." << std::endl;
		}
	}

	return 0;
}

DWORD WINAPI RunClient(ThreadParamsReaderWriter* threadParams)
{
	return RunClient(threadParams->readerWriter, threadParams->criticalSection);
}

DWORD WINAPI RunClient(ThreadParamsPipeName* threadParams)
{
	auto clientPipe = BidirectionalPipe::ClientPipe(threadParams->pipeName);
	return RunClient(clientPipe, threadParams->criticalSection);
}

DWORD WINAPI RunServer(IReaderWriter& readerWriter, AutoCriticalSection& criticalSection, bool silent =false)
{
	EncryptedReaderWriter pipe(readerWriter, true);

	std::tstring data(sampleMessageText);

	if (!silent)
	{
		auto ecs = EnterAutoCriticalSection(criticalSection);
		std::tcout << "SERVER: Sending data to pipe..." << std::endl;
		std::tcout << "SERVER: Message: " << data << std::endl;
	}

	DWORD numBytesWritten = pipe.Write((BYTE*)&data[0], data.length() * sizeof(data[0]));

	if (!silent)
	{
		auto ecs = EnterAutoCriticalSection(criticalSection);
		std::tcout << "SERVER: Number of bytes sent: " << numBytesWritten << std::endl;
		std::tcout << "SERVER: Done." << std::endl;
	}

	return 0;
}

DWORD WINAPI RunServer(ThreadParamsReaderWriter* threadParams)
{
	return RunServer(threadParams->readerWriter, threadParams->criticalSection);
}

DWORD WINAPI RunServer(ThreadParamsPipeName* threadParams)
{
	auto clientPipe = BidirectionalPipe::ServerPipe(threadParams->pipeName);
	return RunServer(clientPipe, threadParams->criticalSection);
}

DWORD WINAPI ClientPipeNameThreadProc(LPVOID lpParameter)
{
	return RunClient((ThreadParamsPipeName*)lpParameter);
}

DWORD WINAPI ClientReaderWriterThreadProc(LPVOID lpParameter)
{
	return RunClient((ThreadParamsReaderWriter*)lpParameter);
}

DWORD WINAPI ServerPipeNameThreadProc(LPVOID lpParameter)
{
	Sleep(1000);
	return RunServer((ThreadParamsPipeName*)lpParameter);
}

DWORD WINAPI ServerReaderWriterThreadProc(LPVOID lpParameter)
{
	Sleep(1000);
	return RunServer((ThreadParamsReaderWriter*)lpParameter);
}

void WaitForThread(DWORD threadId, DWORD milliseconds)
{
	AutoClosableHandle thread(OpenThread(THREAD_ALL_ACCESS, FALSE, threadId));
	if (WAIT_OBJECT_0 != WaitForSingleObject(thread, milliseconds))
	{
		TerminateThread(thread, 0);
	}
}

inline bool begins_with(const std::tstring& text, const std::tstring& prefix)
{
	return text.substr(0, prefix.length()) == prefix;
}

#define THREAD_WAIT_SECONDS (500)

static std::tstring spawnboth(TEXT("spawnboth:"));
static std::tstring threadboth(TEXT("threadboth:"));

static std::tstring pipeserver(TEXT("pipeserver:"));
static std::tstring pipeclient(TEXT("pipeclient:"));

static std::tstring parentclient(TEXT("parentclient:"));
static std::tstring parentserver(TEXT("parentserver:"));

static std::tstring childserver(TEXT("childserver:"));
static std::tstring childclient(TEXT("childclient:"));

int _tmain(int argc, _TCHAR* argv[])
{
	try
	{
		auto criticalSection = CreateAutoCriticalSection();
		if (argc > 1)
		{
			std::tstring arg1(argv[1]);

			MessageBox(NULL, arg1.c_str(), arg1.c_str(), MB_OK);

			if (begins_with(arg1, threadboth))
			{
				auto pipeParameter = arg1.substr(threadboth.length());

				DWORD clientThreadId;
				ThreadParamsPipeName clientThreadParams(pipeParameter, criticalSection);
				AutoClosableHandle clientThread(CreateThread(NULL, 0, ClientPipeNameThreadProc, LPVOID(&clientThreadParams), 0, &clientThreadId));

				DWORD serverThreadId;
				ThreadParamsPipeName serverThreadParams(pipeParameter, criticalSection);
				AutoClosableHandle serverThread(CreateThread(NULL, 0, ServerPipeNameThreadProc, LPVOID(&serverThreadParams), 0, &serverThreadId));

				WaitForThread(clientThreadId, THREAD_WAIT_SECONDS * 1000);
				WaitForThread(serverThreadId, THREAD_WAIT_SECONDS * 1000);
			}
			else if (begins_with(arg1, spawnboth))
			{
				auto pipeParameter = arg1.substr(spawnboth.length());

				TCHAR currentProcessImagePath[MAX_PATH];
				GetModuleFileName(NULL, currentProcessImagePath, rangeof(currentProcessImagePath));

				ChildProcess client(std::tstring(currentProcessImagePath), std::vector<std::tstring>(1, pipeclient + pipeParameter));
				ChildProcess server(std::tstring(currentProcessImagePath), std::vector<std::tstring>(1, pipeserver + pipeParameter));

				client.Wait(INFINITE);
				server.Wait(INFINITE);
			}
			else if (begins_with(arg1, pipeserver))
			{
				auto pipeParameter = arg1.substr(pipeserver.length());
				auto serverPipe = BidirectionalPipe::ServerPipe(pipeParameter);
				return RunServer(serverPipe, criticalSection);
			}
			else if (begins_with(arg1, pipeclient))
			{
				auto pipeParameter = arg1.substr(pipeclient.length());
				auto clientPipe = BidirectionalPipe::ClientPipe(pipeParameter);
				return RunClient(clientPipe, criticalSection);
			}
			else if (begins_with(arg1, parentclient))
			{
				auto pipeParameter = arg1.substr(parentclient.length());

				TCHAR currentProcessImagePath[MAX_PATH];
				GetModuleFileName(NULL, currentProcessImagePath, rangeof(currentProcessImagePath));
				ChildProcess serverChildProcess(std::tstring(currentProcessImagePath), std::vector<std::tstring>(1, childserver + pipeParameter));

				DWORD clientThreadId;
				ThreadParamsReaderWriter threadParams(serverChildProcess, criticalSection);
				AutoClosableHandle pipeClientThread(CreateThread(NULL, 0, ClientReaderWriterThreadProc, LPVOID(&threadParams), 0, &clientThreadId));

				WaitForThread(clientThreadId, THREAD_WAIT_SECONDS * 1000);
				serverChildProcess.Wait(INFINITE);
			}
			else if (begins_with(arg1, parentserver))
			{
				auto pipeParameter = arg1.substr(parentserver.length());

				TCHAR currentProcessImagePath[MAX_PATH];
				GetModuleFileName(NULL, currentProcessImagePath, rangeof(currentProcessImagePath));
				ChildProcess clientChildProcess(std::tstring(currentProcessImagePath), std::vector<std::tstring>(1, childclient + pipeParameter));

				DWORD serverThreadId;
				ThreadParamsReaderWriter threadParams(clientChildProcess, criticalSection);
				AutoClosableHandle pipeServerThread(CreateThread(NULL, 0, ServerReaderWriterThreadProc, LPVOID(&threadParams), 0, &serverThreadId));

				WaitForThread(serverThreadId, THREAD_WAIT_SECONDS * 1000);
				clientChildProcess.Wait(INFINITE);
			}
			else if (begins_with(arg1, childserver))
			{
				StdioReaderWriter readerWriter;
				RunServer(readerWriter, criticalSection, true);
			}
			else if (begins_with(arg1, childclient))
			{
				StdioReaderWriter readerWriter;
				RunClient(readerWriter, criticalSection, true);
			}
		}
	}
	catch(std::string message)
	{
		std::cout << message << std::endl;
	}
	catch(std::wstring message)
	{
		std::wcout << message << std::endl;
	}
	catch(LPCSTR message)
	{
		std::cout << message << std::endl;
	}
	catch(LPCWSTR message)
	{
		std::wcout << message << std::endl;
	}

	return 0;
}