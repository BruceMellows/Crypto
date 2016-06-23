// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#pragma once

#include "tstring.h"
#include "IReaderWriter.h"

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
