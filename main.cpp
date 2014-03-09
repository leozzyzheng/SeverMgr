#include <windows.h>
#include <tchar.h>
#include <stdio.h> 
#include <strsafe.h>
#include <process.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>

#define BUFSIZE 4096 

HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;
PROCESS_INFORMATION g_sChildProc;

void CreateChildProcess(void); 
void WriteToPipe(const char *); 
void ReadFromPipe(void*); 
void ErrorExit(PTSTR); 
BOOL findProcess(DWORD pid);

int _tmain(int argc, TCHAR *argv[]) 
{ 
	SECURITY_ATTRIBUTES saAttr; 

	printf("\n->Start of parent execution.\n");

	// Set the bInheritHandle flag so pipe handles are inherited. 

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL; 

	// Create a pipe for the child process's STDOUT. 

	if ( ! CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0) ) 
		ErrorExit(TEXT("StdoutRd CreatePipe")); 

	// Ensure the read handle to the pipe for STDOUT is not inherited.

	if ( ! SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT) )
		ErrorExit(TEXT("Stdout SetHandleInformation")); 

	// Create a pipe for the child process's STDIN. 

	if (! CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0)) 
		ErrorExit(TEXT("Stdin CreatePipe")); 

	// Ensure the write handle to the pipe for STDIN is not inherited. 

	if ( ! SetHandleInformation(g_hChildStd_IN_Rd, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT) )
		ErrorExit(TEXT("Stdin SetHandleInformation"));

	// Create the child process. 

	CreateChildProcess();

	// Get a handle to an input file for the parent. 
	// This example assumes a plain text file and uses string output to verify data flow.

	// Read from pipe that is the standard output for child process. 

	printf( "\n->Contents of child process STDOUT:\n\n");
	HANDLE readTh = (HANDLE)_beginthread(ReadFromPipe,NULL,NULL);

	char getc;
	std::string buff;
	
	while(!findProcess(g_sChildProc.dwProcessId))
	{
		Sleep(100);
	}

	for(;;)
	{
		//to do socket receive data
		
		if(!findProcess(g_sChildProc.dwProcessId))
			break;

		getc = getchar();

		if(getc == '\n')
		{
			buff += getc;
			WriteToPipe(buff.c_str());
			buff = "";
			continue;
		}

		buff += getc;
	}

	if(!WaitForSingleObject(readTh,INFINITE))
		printf("\n->End of parent execution.\n");

	// The remaining open handles are cleaned up when this process terminates. 
	// To avoid resource leaks in a larger application, close handles explicitly. 

	CloseHandle(g_hChildStd_IN_Wr);
	return 0; 
} 

void CreateChildProcess()
	// Create a child process that uses the previously created pipes for STDIN and STDOUT.
{ 
	TCHAR szCmdline[]=TEXT("java -Xms512M -Xmx1024M -jar craftbukkit-1.6.2-R1.0.jar -nojline 2>&1");
	PROCESS_INFORMATION piProcInfo; 
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE; 

	// Set up members of the PROCESS_INFORMATION structure. 

	ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );

	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection.

	ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
	siStartInfo.cb = sizeof(STARTUPINFO); 
	siStartInfo.hStdError = g_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.hStdInput = g_hChildStd_IN_Rd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	// Create the child process. 

	bSuccess = CreateProcess(NULL, 
		szCmdline,     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		0,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION 

	// If an error occurs, exit the application. 
	if ( ! bSuccess ) 
		ErrorExit(TEXT("CreateProcess"));
	else 
	{
		// Close handles to the child process and its primary thread.
		// Some applications might keep these handles to monitor the status
		// of the child process, for example. 

		g_sChildProc = piProcInfo;
		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);
	}
}

void WriteToPipe(const char * pCmd) 

	// Read from a file and write its contents to the pipe for the child's STDIN.
	// Stop when there is no more data. 
{ 
	DWORD dwRead, dwWritten;
	BOOL bSuccess = FALSE;

	bSuccess = WriteFile(g_hChildStd_IN_Wr, pCmd, strlen(pCmd), &dwWritten, NULL);

	if ( ! bSuccess )
	{
		ErrorExit(TEXT("writeto"));
	}

	// Close the pipe handle so the child process stops reading. 

	//if ( ! CloseHandle(g_hChildStd_IN_Wr) ) 
	//	ErrorExit(TEXT("StdInWr CloseHandle")); 
} 

DWORD dwRead, dwWritten; 
CHAR chBuf[BUFSIZE]; 
BOOL bSuccess = FALSE;
HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
BOOL Goon = false;
HANDLE innerThread = NULL;

void __cdecl readThread(void *)
{
	while (true)
	{
		bSuccess = ReadFile( g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
		SuspendThread(innerThread);
	}
}

BOOL findProcess(DWORD pid)
{
	PROCESSENTRY32 pe32;
	HANDLE hSnaphot;
	HANDLE hApp;

	hSnaphot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0); // 获取进程快照

	if(hSnaphot == INVALID_HANDLE_VALUE)
		return false;

	pe32.dwSize = sizeof( PROCESSENTRY32 );

	if(!Process32First(hSnaphot, &pe32))
		ErrorExit(TEXT("pro")); // 指向第一个进程
	do
	{
		if (pe32.th32ProcessID == pid)
		{
			return true;
		}
	}while(Process32Next(hSnaphot, &pe32)); // 不断循环直到取不到进程

	return false;
}

void __cdecl ReadFromPipe(void*) 

	// Read output from the child process's pipe for STDOUT
	// and write to the parent process's pipe for STDOUT. 
	// Stop when there is no more data. 
{ 
	bool process = true;

	for (;process;) 
	{
		ZeroMemory(&chBuf, sizeof(chBuf));
		dwRead = 0;

		if(innerThread == NULL)
			innerThread = (HANDLE)_beginthread(readThread,NULL,NULL);
		else
		{
			ResumeThread(innerThread);
		}

		while(dwRead == 0)
		{
			if(!findProcess(g_sChildProc.dwProcessId))
			{
				if(bSuccess == false)
					CloseHandle(innerThread);

				innerThread = NULL;
				process = false;
				break;
			}

			Sleep(100);
		}

		bSuccess = WriteFile(hParentStdOut, chBuf, 
			dwRead, &dwWritten, NULL);
		if (! bSuccess || dwRead == 0 ) break; 
	} 
} 

void ErrorExit(PTSTR lpszFunction) 

	// Format a readable error message, display a message box, 
	// and exit from the application.
{ 
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError(); 

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0, NULL );

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
		(lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR)); 
	StringCchPrintf((LPTSTR)lpDisplayBuf, 
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"), 
		lpszFunction, dw, lpMsgBuf); 
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(1);
}

