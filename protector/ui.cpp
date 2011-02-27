#include "common.h"
#include "protector.h"

///////////////////////////////////////////////////////////////////////////////
// Globals

BYTE abHeapLockerShellcode[35] = {
	0x59,								// pop ecx
	0x33, 0xC0,							// xor eax, eax
	0x50,								// push eax
	0x50,								// push eax
	0x51,								// push ecx
	0x68, 0x67, 0x45, 0x23, 0x01,		// push 0x01234567     ; address of EntryPoint
	0x50,								// push eax
	0x50,								// push eax
	0xB8, 0xEF, 0xCD, 0xAB, 0x89,		// mov eax, 0x89ABCDEF ; CreateThread
	0xFF, 0xD0,							// call eax
	0xB8, 0x67, 0x45, 0x23, 0x01,		// mov eax, 0x01234567 ; GetCurrentThread
	0xFF, 0xD0,							// call eax
	0x50,								// push eax
	0xB8, 0x67, 0x45, 0x23, 0x01,		// mov eax, 0x01234567 ; SuspendThread
	0xFF, 0xD0							// call eax
};

///////////////////////////////////////////////////////////////////////////////
// Local prototypes
void SuspendThreadsOfCurrentProcessExceptCurrentThread(BOOL bSuspend);
DWORD WINAPI DisplayMessageBox(LPVOID lpvArgument);
DWORD WINAPI EntryPoint(LPVOID lpvArgument);

///////////////////////////////////////////////////////////////////////////////
// Functions

/******************************************************************************
 * Suspend or resume all threads in the current process except this one
 * @param bSuspend if TRUE, suspend all threads, else resume them
 ******************************************************************************/
void SuspendThreadsOfCurrentProcessExceptCurrentThread(BOOL bSuspend)
{
	HANDLE hThreadSnap;
	THREADENTRY32 sTE32 = {0};
	BOOL bLoop;
	
	// Suspended threads increments a suspend count, so no need to keep track of which thread were
	// already suspended
	__try
	{
		hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		if (INVALID_HANDLE_VALUE == hThreadSnap)
		{
			PrintError(_TEXT("CreateToolhelp32Snapshot failed"));
			return;
		}

		sTE32.dwSize = sizeof(sTE32);

		for (bLoop = Thread32First(hThreadSnap, &sTE32); bLoop; bLoop = Thread32Next(hThreadSnap, &sTE32))
		{
			if (GetCurrentProcessId() == sTE32.th32OwnerProcessID && GetCurrentThreadId() != sTE32.th32ThreadID)
			{
				HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, sTE32.th32ThreadID);
				if (hThread != NULL)
				{
					DWORD dwError = 0;
					if (bSuspend)
					{
						PrintInfo(_TEXT("Suspending thread %08X"), sTE32.th32ThreadID);
						dwError = SuspendThread(hThread);
					}
					else
					{
						PrintInfo(_TEXT("Resuming thread %08X"), sTE32.th32ThreadID);
						dwError = ResumeThread(hThread);
					}
					if (dwError = -1)
					{
						PrintError(_TEXT("Thread suspend/resume failed on thread %08X"), sTE32.th32ThreadID);
					}
					CloseHandle(hThread);
				}
				else
				{
					PrintError(_TEXT("OpenThread failed"));
				}
			}
		}

		CloseHandle(hThreadSnap);
	}
	__except(1)
	{
		PrintError("Exception");
		return;
	}

	return;
}


/******************************************************************************
 * Display a message box that asks if the process should be terminated
 * @param lpvArgument	The string to display
 ******************************************************************************/
DWORD WINAPI DisplayMessageBox(LPVOID lpvArgument)
{
	BOOL bTerminate = FALSE;

	// Check if we should force the termination or ask the user
	if (sHeapLockerSettings.dwForceTermination)
	{
		// Inform the user we are forcing termination
		MessageBox(NULL, (LPCSTR)lpvArgument, MESSAGEBOX_TITLE, MB_OK | MB_ICONSTOP);
		bTerminate = TRUE;
	}
	else 
	{
		// Ask the user if we should force termination
		if (IDYES == MessageBox(NULL, (LPCSTR)lpvArgument, MESSAGEBOX_TITLE, MB_YESNO | MB_ICONEXCLAMATION))
		{
			bTerminate = TRUE;
		}
	}

	// Terminate the process if needed
	if (bTerminate)
	{
		if (!TerminateProcess(GetCurrentProcess(), 0))
		{
			PrintError(_TEXT("TerminateProcess failed"));
			return FALSE;
		}
	}

	return TRUE;
}


/******************************************************************************
 * Suspend all threads, but create a message box in an unsuspended thread
 ******************************************************************************/
BOOL ThreadedMessageBox(LPTSTR pszOutput)
{
	HANDLE hThreadMessageBox;
	DWORD dwExitCode;

	hThreadMessageBox = CreateThread(NULL, 0, DisplayMessageBox, pszOutput, 0, NULL);
	// TODO MUST do something cleaner here instead of a sleep
	Sleep(100);
	// Suspend all threads
	SuspendThreadsOfCurrentProcessExceptCurrentThread(TRUE);
	ResumeThread(hThreadMessageBox);
	dwExitCode = STILL_ACTIVE;
	while (STILL_ACTIVE == dwExitCode)
	{
		GetExitCodeThread(hThreadMessageBox, &dwExitCode);
		Sleep(500);
	}
	CloseHandle(hThreadMessageBox);
	// Resume all threads
	SuspendThreadsOfCurrentProcessExceptCurrentThread(FALSE);
	return FALSE;
}


/******************************************************************************
 * Shell code gets alloc'd in areas likely to be hit by an exploit and will cause
 * this function to run
 ******************************************************************************/
DWORD WINAPI EntryPoint(LPVOID lpvArgument)
{
	TCHAR szOutput[256];

	SuspendThreadsOfCurrentProcessExceptCurrentThread(TRUE);
	_sntprintf_s(szOutput, countof(szOutput), _TRUNCATE, _TEXT("This document is malicious!\nClick OK to terminate this program (%s).\n\nTechnical details: shellcode trap\nReturn address: %08x"), NULL2EmptyString(GetExecutableName()), lpvArgument);
	MessageBox(NULL, szOutput, MESSAGEBOX_TITLE, MB_ICONSTOP);
	TerminateProcess(GetCurrentProcess(), 0);
	return 0;
}


/******************************************************************************
 * Alloc's our own shell code to display a message, in area likely to be hit by
 * an exploit
 ******************************************************************************/
PBYTE ShellCodeToEntryPoint(void)
{
	static PBYTE pbAddress = NULL;
	LPVOID lpvPage;
	MEMORY_BASIC_INFORMATION sMBI = {0};
	unsigned int uiIter;
	DWORD dwOldProtect;
	
	if (NULL != pbAddress)
	{
		// This function has already been set
		return pbAddress;
	}
	
	// TODO MUST I don't like this at all. It's trying to alloc mem at a random address, until it finds a place that works
	srand((unsigned int)time(0));
	for (lpvPage = NULL; NULL == lpvPage;)
	{
		pbAddress = (PBYTE)((rand() % 0x7E + 1) * 0x1000000 + rand() % 0x7F * 0x10000 + rand() % 0x7F * 0x100 + (rand() % 0x7F & 0xFC));
		lpvPage = VirtualAlloc(pbAddress, sizeof(abHeapLockerShellcode), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	}
	if (FUNCTION_FAILED(VirtualQuery(lpvPage, &sMBI, sizeof(sMBI))))
	{
		PrintError(_TEXT("VirtualQuery"));
		return NULL;
	}
	// Copy in the shellcode
	for (uiIter = 0; uiIter < sizeof(abHeapLockerShellcode); uiIter++)
	{
		*(pbAddress + uiIter) = abHeapLockerShellcode[uiIter];
	}
	// Copy in the variable values
	*(unsigned int *)(pbAddress + INDEX_ENTRYPOINT) = (unsigned int)EntryPoint;
	*(unsigned int *)(pbAddress + INDEX_CREATETHREAD) = (unsigned int)CreateThread;
	*(unsigned int *)(pbAddress + INDEX_GETCURRENTTHREAD) = (unsigned int)GetCurrentThread;
	*(unsigned int *)(pbAddress + INDEX_SUSPENDTHREAD) = (unsigned int)SuspendThread;
	if (FUNCTION_FAILED(VirtualProtect(lpvPage, sMBI.RegionSize, PAGE_EXECUTE, &dwOldProtect)))
	{
		PrintError(_TEXT("VirtualProtect failed"));
		return NULL;
	}
	if (sHeapLockerSettings.dwVerbose > 0)
		PrintInfo(_TEXT("Shellcode address = %08x"), pbAddress);
	return pbAddress;
}