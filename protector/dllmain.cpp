/**
 * This file is part of OpenHIPS.
 *
 * OpenHIPS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenHIPS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenHIPS.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file		protector\dllmain.cpp
 * @summary		Based heavily on Didier Steven's HeapLocker project
 *		(http://blog.didierstevens.com/programs/heaplocker/)
 *		Based on HeapLocker 0.0.0.3 from 2010/12/15
 */

#include "common.h"
#include "protector.h"

#pragma comment(lib, "psapi.lib")

///////////////////////////////////////////////////////////////////////////////
// Globals

HEAPLOCKER_SETTINGS sHeapLockerSettings;

BYTE abHeapLockerShellcode[] = {
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

BYTE abNOPs[] = {
	0x27, // daa
	0x2F, // das
	0x37, // aaa
	0x3F, // aas
	0x40, // inc eax
	0x41, // inc ecx
	0x42, // inc edx
	0x43, // inc ebx
	0x45, // inc ebp
	0x46, // inc esi
	0x47, // inc edi
	0x48, // dec eax
	0x49, // dec ecx
	0x4A, // dec edx
	0x4B, // dec ebx
	0x4D, // dec ebp
	0x4E, // dec esi
	0x4F, // dec edi
	0x90, // nop
	0x91, // xchg    eax, ecx
	0x92, // xchg    eax, edx
	0x93, // xchg    eax, ebx
	0x95, // xchg    eax, ebp
	0x96, // xchg    eax, esi
	0x97, // xchg    eax, edi
	0x98, // cwde
	0x99, // cdq
	0x9E, // sahf
	0x9F, // lahf
	0xD6, // setalc
	0xF5, // cmc
	0xF8, // clc
	0xF9, // stc
	0xFC, // cld
	0xFD, // std
};

BYTE abNOPSledDetection[256];

static TCHAR szModuleName[MAX_PATH];
static TCHAR szDump[256];

///////////////////////////////////////////////////////////////////////////////
// Functions

/******************************************************************************
 * @return An empty string if pszString is NULL
 ******************************************************************************/
LPTSTR NULL2EmptyString(LPTSTR pszString)
{
	return pszString == NULL ? "" : pszString;
}


/******************************************************************************
 * @return An empty string if pszString is NULL
 ******************************************************************************/
LPTSTR GetExecutableName(void)
{
	LPTSTR pszEXE;

	__try
	{
		if (!GetModuleFileName(NULL, szModuleName, MAX_PATH))
		{
			PrintError(_TEXT("GetModuleFileName"));
			return NULL;
		}
		pszEXE = _tcsrchr(szModuleName, '\\');
		if (NULL == pszEXE)
		{
			return NULL;
		}
		return ++pszEXE;
	}
	__except(1)
	{
		PrintError("Exception");
		return NULL;
	}
}


/******************************************************************************
 * @return Returns a pointer to a global var containing a hexdump of the given values
 ******************************************************************************/
LPTSTR HexDump(PBYTE pbFound, int iSize)
{
	int iIter;

	__try
	{
		szDump[iSize] = '\0';
		for (iIter = 0; iIter < iSize && iIter < countof(szDump); iIter++)
		{
			szDump[iIter] = isprint(*(pbFound + iIter*2)) ? *(pbFound + iIter*2) : '.';
		}
		PrintInfo(_TEXT(" %08X: %s"), pbFound, szDump);
		return szDump;
	}
	__except(1)
	{
		PrintError("Exception");
	}

	return NULL;
}


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


/******************************************************************************
 * Fills a memory page with calls to our shell-code
 ******************************************************************************/
SIZE_T PreallocateAddress(DWORD dwAddress, int iMode)
{
	MEMORY_BASIC_INFORMATION sMBI = {0};
	LPVOID lpvPage;
	PBYTE pbAddress;
	unsigned int uiIter;
	DWORD dwOldProtect;
	SIZE_T stReturn;

	__try
	{
		stReturn = 0;
		pbAddress = (PBYTE)dwAddress;
		lpvPage = VirtualAlloc((LPVOID)pbAddress, sizeof(abHeapLockerShellcode), MEM_COMMIT | MEM_RESERVE, ADDRESS_MODE_NOACCESS == iMode ? PAGE_NOACCESS : PAGE_EXECUTE_READWRITE);
		if (NULL == lpvPage)
		{
			if (sHeapLockerSettings.dwVerbose > 0)
			{
				PrintError(_TEXT("VirtualAlloc failed"));
			}
			return stReturn;
		}
		if (FUNCTION_FAILED(VirtualQuery(lpvPage, &sMBI, sizeof(sMBI))))
		{
			PrintError(_TEXT("VirtualQuery failed"));
			return stReturn;
		}
		if (sHeapLockerSettings.dwVerbose > 0)
		{
			PrintInfo(_TEXT("Exploit address = %08x mode = %d page address = %08x memory size = %ld"), pbAddress, iMode, lpvPage, sMBI.RegionSize);
		}
		stReturn = sMBI.RegionSize;
		if (ADDRESS_MODE_NOACCESS != iMode)
		{
			srand((unsigned int)time(0));
			// Fill region with nop-sled
			for (uiIter = 0; uiIter < sMBI.RegionSize; uiIter++)
			{
				*((PBYTE)lpvPage + uiIter) = abNOPs[rand() % countof(abNOPs)];
			}
			// TODO MUST Why is this calling rand()?
			pbAddress = (PBYTE)lpvPage + sMBI.RegionSize - 7 - rand() % 0x10;
			*pbAddress = 0xB8; // mov eax
			*(unsigned int *)(pbAddress + 1) = (unsigned int)ShellCodeToEntryPoint();
			*(pbAddress + 5) = 0xFF; // call
			*(pbAddress + 6) = 0xD0; // eax
			if (FUNCTION_FAILED(VirtualProtect(lpvPage, sMBI.RegionSize, PAGE_EXECUTE, &dwOldProtect)))
			{
				PrintError(_TEXT("VirtualProtect"));
				return stReturn;
			}
		}
	}
	__except(1)
	{
		PrintError("Exception");
		return stReturn;
	}

	return stReturn;
}


/******************************************************************************
 * Reads from the registry to determine if memory for the given process should
 * be protected, and if so protects it.
 ******************************************************************************/
void ProtectAddresses(HKEY hKeyApplication)
{
	HKEY hKey;
	DWORD dwValue;
	DWORD dwValueSize;
	DWORD dwType;
	DWORD dwIndex;
	TCHAR szValueName[MAX_REGISTRY_VALUE_NAME];
	DWORD dwValueNameSize;
	SIZE_T stMemory;
	int iIter;

	stMemory = 0;
	if (NULL != hKeyApplication && IS_SUCCESS(RegOpenKeyEx(hKeyApplication, REGISTRY_ADDRESSES, 0L, KEY_READ, &hKey)))
	{
		dwIndex = 0;
		dwValueNameSize = countof(szValueName);
		dwValueSize = sizeof(dwValue);
		while (ERROR_NO_MORE_ITEMS != RegEnumValue(hKey, dwIndex++, szValueName, &dwValueNameSize, NULL, &dwType, (LPBYTE) &dwValue, &dwValueSize))
		{
			if (REG_DWORD == dwType)
			{
				if (sHeapLockerSettings.dwVerbose > 0)
				{
					PrintInfo(_TEXT("Pre-allocating page for address 0x%08x (%s)"), dwValue, szValueName);
				}
				if (isdigit(szValueName[0]))
				{
					stMemory += PreallocateAddress(dwValue, szValueName[0] - '0');
				}
				else
				{
					stMemory += PreallocateAddress(dwValue, ADDRESS_MODE_NOACCESS);
				}
			}
			dwValueNameSize = countof(szValueName);
			dwValueSize = sizeof(dwValue);
		}
		RegCloseKey(hKey);
	}
	if (sHeapLockerSettings.dwGenericPreAllocate != 0xFFFFFFFF)
	{
		for (iIter = 1; iIter < 128; iIter++)
		{
			stMemory += PreallocateAddress(iIter * 0x1000000 + iIter * 0x10000 + iIter * 0x100 + iIter, sHeapLockerSettings.dwGenericPreAllocate);
		}
	}
	if (sHeapLockerSettings.dwVerbose > 0)
	{
		PrintInfo(_TEXT("Memory used for pre-allocated pages = %ld KB"), stMemory / 1024);
	}
}


/******************************************************************************
 * Read application specific registry settings
 ******************************************************************************/
void ReadHeapLockerSettingsFromRegistryApplication(HKEY hKeyApplication)
{
	DWORD dwType;
	DWORD dwValue;
	DWORD dwValueSize;
	BYTE abValue[1025];
	int iIter;

	dwValueSize = sizeof(dwValue);
	if (IS_SUCCESS(RegQueryValueEx(hKeyApplication, REGISTRY_PRIVATE_USAGE_MAX, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
	{
		if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwPrivateUsageMax < ORIGIN_REGISTRY_APPLICATION)
		{
			sHeapLockerSettings.iOrigin_dwPrivateUsageMax = ORIGIN_REGISTRY_APPLICATION;
			sHeapLockerSettings.dwPrivateUsageMax = dwValue;
		}
	}

	dwValueSize = sizeof(dwValue);
	if (IS_SUCCESS(RegQueryValueEx(hKeyApplication, REGISTRY_NOP_SLED_LENGTH_MAX, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
	{
		if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwNOPSledLengthMin < ORIGIN_REGISTRY_APPLICATION)
		{
			sHeapLockerSettings.iOrigin_dwNOPSledLengthMin = ORIGIN_REGISTRY_APPLICATION;
			sHeapLockerSettings.dwNOPSledLengthMin = dwValue;
		}
	}

	dwValueSize = sizeof(dwValue);
	if (IS_SUCCESS(RegQueryValueEx(hKeyApplication, REGISTRY_GENERIC_PRE_ALLOCATE, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
	{
		if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwGenericPreAllocate < ORIGIN_REGISTRY_APPLICATION)
		{
			sHeapLockerSettings.iOrigin_dwGenericPreAllocate = ORIGIN_REGISTRY_APPLICATION;
			sHeapLockerSettings.dwGenericPreAllocate = dwValue;
		}
	}

	dwValueSize = sizeof(dwValue);
	if (IS_SUCCESS(RegQueryValueEx(hKeyApplication, REGISTRY_VERBOSE, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
	{
		if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwVerbose < ORIGIN_REGISTRY_APPLICATION)
		{
			sHeapLockerSettings.iOrigin_dwVerbose = ORIGIN_REGISTRY_APPLICATION;
			sHeapLockerSettings.dwVerbose = dwValue;
		}
	}

	dwValueSize = sizeof(dwValue);
	if (IS_SUCCESS(RegQueryValueEx(hKeyApplication, REGISTRY_SEARCH_MODE, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
	{
		if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_iSearchMode < ORIGIN_REGISTRY_APPLICATION)
		{
			sHeapLockerSettings.iOrigin_iSearchMode = ORIGIN_REGISTRY_APPLICATION;
			sHeapLockerSettings.iSearchMode = dwValue;
		}
	}

	dwValueSize = sizeof(abValue);
	if (IS_SUCCESS(RegQueryValueEx(hKeyApplication, REGISTRY_SEARCH_STRING, 0, &dwType, abValue, &dwValueSize)))
	{
		if (REG_BINARY == dwType && sHeapLockerSettings.iOrigin_iSearchLen < ORIGIN_REGISTRY_APPLICATION)
		{
			sHeapLockerSettings.iOrigin_iSearchLen = ORIGIN_REGISTRY_APPLICATION;
			sHeapLockerSettings.iSearchLen = dwValueSize;
			for (iIter = 0; iIter < sHeapLockerSettings.iSearchLen; iIter++)
				sHeapLockerSettings.abSearch[iIter] = abValue[iIter];
		}
	}

	dwValueSize = sizeof(dwValue);
	if (IS_SUCCESS(RegQueryValueEx(hKeyApplication, REGISTRY_NULL_PAGE_PRE_ALLOCATE, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
	{
		if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwPreallocatePage0 < ORIGIN_REGISTRY_APPLICATION)
		{
			sHeapLockerSettings.iOrigin_dwPreallocatePage0 = ORIGIN_REGISTRY_APPLICATION;
			sHeapLockerSettings.dwPreallocatePage0 = dwValue;
		}
	}

	dwValueSize = sizeof(dwValue);
	if (IS_SUCCESS(RegQueryValueEx(hKeyApplication, REGISTRY_FORCE_TERMINATION, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
	{
		if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwForceTermination < ORIGIN_REGISTRY_APPLICATION)
		{
			sHeapLockerSettings.iOrigin_dwForceTermination = ORIGIN_REGISTRY_APPLICATION;
			sHeapLockerSettings.dwForceTermination = dwValue;
		}
	}

	dwValueSize = sizeof(dwValue);
	if (IS_SUCCESS(RegQueryValueEx(hKeyApplication, REGISTRY_RESUME_MONITORING, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
	{
		if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwResumeMonitoring < ORIGIN_REGISTRY_APPLICATION)
		{
			sHeapLockerSettings.iOrigin_dwResumeMonitoring = ORIGIN_REGISTRY_APPLICATION;
			sHeapLockerSettings.dwResumeMonitoring = dwValue;
		}
	}
}


/******************************************************************************
 * Read generic HeapLocker settings from the registry
 ******************************************************************************/
void ReadHeapLockerSettingsFromRegistry(void)
{
	HKEY hKey;
	DWORD dwType;
	DWORD dwValue;
	DWORD dwValueSize;
	BYTE abValue[1025];
	int iIter;

	if (IS_SUCCESS(RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_PATH, 0L, KEY_READ, &hKey)))
	{
		dwValueSize = sizeof(dwValue);
		if (IS_SUCCESS(RegQueryValueEx(hKey, REGISTRY_PRIVATE_USAGE_MAX, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
		{
			if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwPrivateUsageMax < ORIGIN_REGISTRY_GENERIC)
			{
				sHeapLockerSettings.iOrigin_dwPrivateUsageMax = ORIGIN_REGISTRY_GENERIC;
				sHeapLockerSettings.dwPrivateUsageMax = dwValue;
			}
		}

		dwValueSize = sizeof(dwValue);
		if (IS_SUCCESS(RegQueryValueEx(hKey, REGISTRY_NOP_SLED_LENGTH_MAX, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
		{
			if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwNOPSledLengthMin < ORIGIN_REGISTRY_GENERIC)
			{
				sHeapLockerSettings.iOrigin_dwNOPSledLengthMin = ORIGIN_REGISTRY_GENERIC;
				sHeapLockerSettings.dwNOPSledLengthMin = dwValue;
			}
		}

		dwValueSize = sizeof(dwValue);
		if (IS_SUCCESS(RegQueryValueEx(hKey, REGISTRY_GENERIC_PRE_ALLOCATE, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
		{
			if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwGenericPreAllocate < ORIGIN_REGISTRY_GENERIC)
			{
				sHeapLockerSettings.iOrigin_dwGenericPreAllocate = ORIGIN_REGISTRY_GENERIC;
				sHeapLockerSettings.dwGenericPreAllocate = dwValue;
			}
		}

		dwValueSize = sizeof(dwValue);
		if (IS_SUCCESS(RegQueryValueEx(hKey, REGISTRY_VERBOSE, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
		{
			if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwVerbose < ORIGIN_REGISTRY_GENERIC)
			{
				sHeapLockerSettings.iOrigin_dwVerbose = ORIGIN_REGISTRY_GENERIC;
				sHeapLockerSettings.dwVerbose = dwValue;
			}
		}

		dwValueSize = sizeof(dwValue);
		if (IS_SUCCESS(RegQueryValueEx(hKey, REGISTRY_SEARCH_MODE, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
		{
			if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_iSearchMode < ORIGIN_REGISTRY_GENERIC)
			{
				sHeapLockerSettings.iOrigin_iSearchMode = ORIGIN_REGISTRY_GENERIC;
				sHeapLockerSettings.iSearchMode = dwValue;
			}
		}

		dwValueSize = sizeof(abValue);
		if (IS_SUCCESS(RegQueryValueEx(hKey, REGISTRY_SEARCH_STRING, 0, &dwType, abValue, &dwValueSize)))
		{
			if (REG_BINARY == dwType && sHeapLockerSettings.iOrigin_iSearchLen < ORIGIN_REGISTRY_GENERIC)
			{
				sHeapLockerSettings.iOrigin_iSearchLen = ORIGIN_REGISTRY_GENERIC;
				sHeapLockerSettings.iSearchLen = dwValueSize;
				for (iIter = 0; iIter < sHeapLockerSettings.iSearchLen; iIter++)
				{
					sHeapLockerSettings.abSearch[iIter] = abValue[iIter];
				}
			}
		}

		dwValueSize = sizeof(dwValue);
		if (IS_SUCCESS(RegQueryValueEx(hKey, REGISTRY_NULL_PAGE_PRE_ALLOCATE, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
		{
			if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwPreallocatePage0 < ORIGIN_REGISTRY_GENERIC)
			{
				sHeapLockerSettings.iOrigin_dwPreallocatePage0 = ORIGIN_REGISTRY_GENERIC;
				sHeapLockerSettings.dwPreallocatePage0 = dwValue;
			}
		}

		dwValueSize = sizeof(dwValue);
		if (IS_SUCCESS(RegQueryValueEx(hKey, REGISTRY_FORCE_TERMINATION, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
		{
			if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwForceTermination < ORIGIN_REGISTRY_GENERIC)
			{
				sHeapLockerSettings.iOrigin_dwForceTermination = ORIGIN_REGISTRY_GENERIC;
				sHeapLockerSettings.dwForceTermination = dwValue;
			}
		}

		dwValueSize = sizeof(dwValue);
		if (IS_SUCCESS(RegQueryValueEx(hKey, REGISTRY_RESUME_MONITORING, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
		{
			if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwResumeMonitoring < ORIGIN_REGISTRY_GENERIC)
			{
				sHeapLockerSettings.iOrigin_dwResumeMonitoring = ORIGIN_REGISTRY_GENERIC;
				sHeapLockerSettings.dwResumeMonitoring = dwValue;
			}
		}

		RegCloseKey(hKey);
	}
}

/******************************************************************************
 * Set default HeapLocker settings
 ******************************************************************************/
void SetHeapLockerSettingsDefaults(void)
{
	if (sHeapLockerSettings.iOrigin_dwPrivateUsageMax < ORIGIN_DEFAULT)
	{
		sHeapLockerSettings.iOrigin_dwPrivateUsageMax = ORIGIN_DEFAULT;
		sHeapLockerSettings.dwPrivateUsageMax = 0xFFFFFFFF;
	}

	if (sHeapLockerSettings.iOrigin_dwNOPSledLengthMin < ORIGIN_DEFAULT)
	{
		sHeapLockerSettings.iOrigin_dwNOPSledLengthMin = ORIGIN_DEFAULT;
		sHeapLockerSettings.dwNOPSledLengthMin = 0;
	}

	if (sHeapLockerSettings.iOrigin_dwGenericPreAllocate < ORIGIN_DEFAULT)
	{
		sHeapLockerSettings.iOrigin_dwGenericPreAllocate = ORIGIN_DEFAULT;
		sHeapLockerSettings.dwGenericPreAllocate = 0xFFFFFFFF;
	}

	if (sHeapLockerSettings.iOrigin_dwVerbose < ORIGIN_DEFAULT)
	{
		sHeapLockerSettings.iOrigin_dwVerbose = ORIGIN_DEFAULT;
		sHeapLockerSettings.dwVerbose = 0;
	}

	if (sHeapLockerSettings.iOrigin_iSearchMode < ORIGIN_DEFAULT)
	{
		sHeapLockerSettings.iOrigin_iSearchMode = ORIGIN_DEFAULT;
		sHeapLockerSettings.iSearchMode = 0;
	}

	if (sHeapLockerSettings.iOrigin_iSearchLen < ORIGIN_DEFAULT)
	{
		sHeapLockerSettings.iOrigin_iSearchLen = ORIGIN_DEFAULT;
		sHeapLockerSettings.iSearchLen = 0;
	}

	if (sHeapLockerSettings.iOrigin_dwPreallocatePage0 < ORIGIN_DEFAULT)
	{
		sHeapLockerSettings.iOrigin_dwPreallocatePage0 = ORIGIN_DEFAULT;
		sHeapLockerSettings.dwPreallocatePage0 = 0;
	}

	if (sHeapLockerSettings.iOrigin_dwForceTermination < ORIGIN_DEFAULT)
	{
		sHeapLockerSettings.iOrigin_dwForceTermination = ORIGIN_DEFAULT;
		sHeapLockerSettings.dwForceTermination = 0;
	}

	if (sHeapLockerSettings.iOrigin_dwResumeMonitoring < ORIGIN_DEFAULT)
	{
		sHeapLockerSettings.iOrigin_dwResumeMonitoring = ORIGIN_DEFAULT;
		sHeapLockerSettings.dwResumeMonitoring = 0;
	}
}


/******************************************************************************
 * Check private memory usage
 ******************************************************************************/
BOOL CheckPrivateUsage(void)
{
	PROCESS_MEMORY_COUNTERS_EX sPMCE;
	TCHAR szOutput[256];

	GetProcessMemoryInfo(GetCurrentProcess(), (PPROCESS_MEMORY_COUNTERS)&sPMCE, sizeof(sPMCE));
	if (sHeapLockerSettings.dwVerbose > 0)
	{
		PrintInfo(_TEXT("PrivateUsage %ld MB"), sPMCE.PrivateUsage / 1024 / 1024);
	}
	//	PrintInfo(_TEXT("Sum %ld MB"), (sPMCE.PrivateUsage + sPMCE.WorkingSetSize + sPMCE.QuotaPagedPoolUsage + sPMCE.QuotaNonPagedPoolUsage + sPMCE.PagefileUsage) / 1024 / 1024);
	if (sPMCE.PrivateUsage / 1024 / 1024 >= sHeapLockerSettings.dwPrivateUsageMax)
	{
		// Application is using more than the configured max, display message box
		if (sHeapLockerSettings.dwForceTermination)
		{
			_sntprintf_s(szOutput, countof(szOutput), _TRUNCATE, _TEXT("This document is probably malicious!\nClick OK to terminate this program (%s).\n\nTechnical details: heap-spray detected\nPrivateUsage %ld MB"), NULL2EmptyString(GetExecutableName()), sPMCE.PrivateUsage / 1024 / 1024);
		}
		else
		{
			_sntprintf_s(szOutput, countof(szOutput), _TRUNCATE, _TEXT("This document is probably malicious!\nDo you want to terminate this program (%s)?\n\nTechnical details: heap-spray detected\nPrivateUsage %ld MB"), NULL2EmptyString(GetExecutableName()), sPMCE.PrivateUsage / 1024 / 1024);
		}

		if (FALSE == ThreadedMessageBox(szOutput))
		{
			return FALSE;
		}
	}
	return TRUE;
}


/******************************************************************************
 * Thread to monitor private memory usage
 ******************************************************************************/
DWORD WINAPI MonitorPrivateUsage(LPVOID lpvArgument)
{
	while(CheckPrivateUsage())
	{
		Sleep(1000);
	}

	return 0;
}


/******************************************************************************
 * Open application registry key
 ******************************************************************************/
HKEY GetApplicationRegKey(void)
{
	LPTSTR pszEXE;
	HKEY hKey;
	HKEY hKeyApplication;
	DWORD dwIndex;
	TCHAR szKeyName[MAX_REGISTRY_KEY_NAME];
	DWORD dwKeyNameSize;

	__try
	{
		pszEXE = GetExecutableName();
		if (NULL == pszEXE)
			return NULL;
		PrintInfo(_TEXT("Application name: %s"), pszEXE);
		if (IS_SUCCESS(RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_PATH_APPLICATIONS, 0L, KEY_READ, &hKey)))
		{
			// TODO SHOULD instead of enumerating the subkeys, just open the key, or return failure if it does not exist
			dwIndex = 0;
			dwKeyNameSize = countof(szKeyName);
			while (ERROR_NO_MORE_ITEMS != RegEnumKeyEx(hKey, dwIndex++, szKeyName, &dwKeyNameSize, NULL, NULL, NULL, NULL))
			{
				if (!_tcsicmp(pszEXE, szKeyName))
				{
					if (IS_SUCCESS(RegOpenKeyEx(hKey, szKeyName, 0L, KEY_READ, &hKeyApplication)))
					{
						PrintInfo(_TEXT("Found registry settings for application %s"), szKeyName);
						RegCloseKey(hKey);
						return hKeyApplication;
					}
				}
				dwKeyNameSize = countof(szKeyName);
			}
			RegCloseKey(hKey);
		}
	}
	__except(1)
	{
		PrintError("Exception");
		return NULL;
	}

	return NULL;
}

/******************************************************************************
 * Init array describing one-byte nop sleds
 ******************************************************************************/
void InitializeabNOPSledDetection(void)
{
	abNOPSledDetection[0x00] = FALSE;
	abNOPSledDetection[0x01] = TRUE;
	abNOPSledDetection[0x02] = TRUE;
	abNOPSledDetection[0x03] = TRUE;
	abNOPSledDetection[0x04] = TRUE;
	abNOPSledDetection[0x05] = TRUE;
	abNOPSledDetection[0x06] = TRUE;
	abNOPSledDetection[0x07] = TRUE;
	abNOPSledDetection[0x08] = TRUE;
	abNOPSledDetection[0x09] = TRUE;
	abNOPSledDetection[0x0A] = TRUE;
	abNOPSledDetection[0x0B] = TRUE;
	abNOPSledDetection[0x0C] = TRUE;
	abNOPSledDetection[0x0D] = TRUE;
	abNOPSledDetection[0x0E] = TRUE;
	abNOPSledDetection[0x0F] = FALSE;
	abNOPSledDetection[0x10] = TRUE;
	abNOPSledDetection[0x11] = TRUE;
	abNOPSledDetection[0x12] = TRUE;
	abNOPSledDetection[0x13] = TRUE;
	abNOPSledDetection[0x14] = TRUE;
	abNOPSledDetection[0x15] = TRUE;
	abNOPSledDetection[0x16] = TRUE;
	abNOPSledDetection[0x17] = TRUE;
	abNOPSledDetection[0x18] = TRUE;
	abNOPSledDetection[0x19] = TRUE;
	abNOPSledDetection[0x1A] = TRUE;
	abNOPSledDetection[0x1B] = TRUE;
	abNOPSledDetection[0x1C] = TRUE;
	abNOPSledDetection[0x1D] = TRUE;
	abNOPSledDetection[0x1E] = TRUE;
	abNOPSledDetection[0x1F] = TRUE;
	abNOPSledDetection[0x20] = TRUE;
	abNOPSledDetection[0x21] = TRUE;
	abNOPSledDetection[0x22] = TRUE;
	abNOPSledDetection[0x23] = TRUE;
	abNOPSledDetection[0x24] = TRUE;
	abNOPSledDetection[0x25] = TRUE;
	abNOPSledDetection[0x26] = FALSE;
	abNOPSledDetection[0x27] = TRUE;
	abNOPSledDetection[0x28] = TRUE;
	abNOPSledDetection[0x29] = TRUE;
	abNOPSledDetection[0x2A] = TRUE;
	abNOPSledDetection[0x2B] = TRUE;
	abNOPSledDetection[0x2C] = TRUE;
	abNOPSledDetection[0x2D] = TRUE;
	abNOPSledDetection[0x2E] = FALSE;
	abNOPSledDetection[0x2F] = TRUE;
	abNOPSledDetection[0x30] = TRUE;
	abNOPSledDetection[0x31] = TRUE;
	abNOPSledDetection[0x32] = TRUE;
	abNOPSledDetection[0x33] = FALSE;
	abNOPSledDetection[0x34] = TRUE;
	abNOPSledDetection[0x35] = TRUE;
	abNOPSledDetection[0x36] = FALSE;
	abNOPSledDetection[0x37] = TRUE;
	abNOPSledDetection[0x38] = TRUE;
	abNOPSledDetection[0x39] = TRUE;
	abNOPSledDetection[0x3A] = TRUE;
	abNOPSledDetection[0x3B] = TRUE;
	abNOPSledDetection[0x3C] = TRUE;
	abNOPSledDetection[0x3D] = TRUE;
	abNOPSledDetection[0x3E] = FALSE;
	abNOPSledDetection[0x3F] = TRUE;
	abNOPSledDetection[0x40] = TRUE;
	abNOPSledDetection[0x41] = TRUE;
	abNOPSledDetection[0x42] = TRUE;
	abNOPSledDetection[0x43] = TRUE;
	abNOPSledDetection[0x44] = TRUE;
	abNOPSledDetection[0x45] = TRUE;
	abNOPSledDetection[0x46] = TRUE;
	abNOPSledDetection[0x47] = TRUE;
	abNOPSledDetection[0x48] = TRUE;
	abNOPSledDetection[0x49] = TRUE;
	abNOPSledDetection[0x4A] = TRUE;
	abNOPSledDetection[0x4B] = TRUE;
	abNOPSledDetection[0x4C] = TRUE;
	abNOPSledDetection[0x4D] = TRUE;
	abNOPSledDetection[0x4E] = TRUE;
	abNOPSledDetection[0x4F] = TRUE;
	abNOPSledDetection[0x50] = TRUE;
	abNOPSledDetection[0x51] = TRUE;
	abNOPSledDetection[0x52] = TRUE;
	abNOPSledDetection[0x53] = TRUE;
	abNOPSledDetection[0x54] = TRUE;
	abNOPSledDetection[0x55] = TRUE;
	abNOPSledDetection[0x56] = TRUE;
	abNOPSledDetection[0x57] = TRUE;
	abNOPSledDetection[0x58] = TRUE;
	abNOPSledDetection[0x59] = TRUE;
	abNOPSledDetection[0x5A] = TRUE;
	abNOPSledDetection[0x5B] = TRUE;
	abNOPSledDetection[0x5C] = TRUE;
	abNOPSledDetection[0x5D] = TRUE;
	abNOPSledDetection[0x5E] = TRUE;
	abNOPSledDetection[0x5F] = TRUE;
	abNOPSledDetection[0x60] = TRUE;
	abNOPSledDetection[0x61] = TRUE;
	abNOPSledDetection[0x62] = TRUE;
	abNOPSledDetection[0x63] = TRUE;
	abNOPSledDetection[0x64] = FALSE;
	abNOPSledDetection[0x65] = FALSE;
	abNOPSledDetection[0x66] = FALSE;
	abNOPSledDetection[0x67] = FALSE;
	abNOPSledDetection[0x68] = TRUE;
	abNOPSledDetection[0x69] = TRUE;
	abNOPSledDetection[0x6A] = TRUE;
	abNOPSledDetection[0x6B] = TRUE;
	abNOPSledDetection[0x6C] = TRUE;
	abNOPSledDetection[0x6D] = TRUE;
	abNOPSledDetection[0x6E] = TRUE;
	abNOPSledDetection[0x6F] = TRUE;
	abNOPSledDetection[0x70] = TRUE;
	abNOPSledDetection[0x71] = TRUE;
	abNOPSledDetection[0x72] = TRUE;
	abNOPSledDetection[0x73] = TRUE;
	abNOPSledDetection[0x74] = TRUE;
	abNOPSledDetection[0x75] = TRUE;
	abNOPSledDetection[0x76] = TRUE;
	abNOPSledDetection[0x77] = TRUE;
	abNOPSledDetection[0x78] = TRUE;
	abNOPSledDetection[0x79] = TRUE;
	abNOPSledDetection[0x7A] = TRUE;
	abNOPSledDetection[0x7B] = TRUE;
	abNOPSledDetection[0x7C] = TRUE;
	abNOPSledDetection[0x7D] = TRUE;
	abNOPSledDetection[0x7E] = TRUE;
	abNOPSledDetection[0x7F] = TRUE;
	abNOPSledDetection[0x80] = TRUE;
	abNOPSledDetection[0x81] = TRUE;
	abNOPSledDetection[0x82] = TRUE;
	abNOPSledDetection[0x83] = TRUE;
	abNOPSledDetection[0x84] = TRUE;
	abNOPSledDetection[0x85] = TRUE;
	abNOPSledDetection[0x86] = TRUE;
	abNOPSledDetection[0x87] = TRUE;
	abNOPSledDetection[0x88] = TRUE;
	abNOPSledDetection[0x89] = TRUE;
	abNOPSledDetection[0x8A] = TRUE;
	abNOPSledDetection[0x8B] = TRUE;
	abNOPSledDetection[0x8C] = TRUE;
	abNOPSledDetection[0x8D] = TRUE;
	abNOPSledDetection[0x8E] = FALSE;
	abNOPSledDetection[0x8F] = FALSE;
	abNOPSledDetection[0x90] = TRUE;
	abNOPSledDetection[0x91] = TRUE;
	abNOPSledDetection[0x92] = TRUE;
	abNOPSledDetection[0x93] = TRUE;
	abNOPSledDetection[0x94] = TRUE;
	abNOPSledDetection[0x95] = TRUE;
	abNOPSledDetection[0x96] = TRUE;
	abNOPSledDetection[0x97] = TRUE;
	abNOPSledDetection[0x98] = TRUE;
	abNOPSledDetection[0x99] = TRUE;
	abNOPSledDetection[0x9A] = FALSE;
	abNOPSledDetection[0x9B] = FALSE;
	abNOPSledDetection[0x9C] = TRUE;
	abNOPSledDetection[0x9D] = TRUE;
	abNOPSledDetection[0x9E] = TRUE;
	abNOPSledDetection[0x9F] = TRUE;
	abNOPSledDetection[0xA0] = FALSE;
	abNOPSledDetection[0xA1] = FALSE;
	abNOPSledDetection[0xA2] = FALSE;
	abNOPSledDetection[0xA3] = FALSE;
	abNOPSledDetection[0xA4] = FALSE;
	abNOPSledDetection[0xA5] = FALSE;
	abNOPSledDetection[0xA6] = FALSE;
	abNOPSledDetection[0xA7] = FALSE;
	abNOPSledDetection[0xA8] = TRUE;
	abNOPSledDetection[0xA9] = TRUE;
	abNOPSledDetection[0xAA] = FALSE;
	abNOPSledDetection[0xAB] = FALSE;
	abNOPSledDetection[0xAC] = FALSE;
	abNOPSledDetection[0xAD] = FALSE;
	abNOPSledDetection[0xAE] = FALSE;
	abNOPSledDetection[0xAF] = FALSE;
	abNOPSledDetection[0xB0] = TRUE;
	abNOPSledDetection[0xB1] = TRUE;
	abNOPSledDetection[0xB2] = TRUE;
	abNOPSledDetection[0xB3] = TRUE;
	abNOPSledDetection[0xB4] = TRUE;
	abNOPSledDetection[0xB5] = TRUE;
	abNOPSledDetection[0xB6] = TRUE;
	abNOPSledDetection[0xB7] = TRUE;
	abNOPSledDetection[0xB8] = TRUE;
	abNOPSledDetection[0xB9] = TRUE;
	abNOPSledDetection[0xBA] = TRUE;
	abNOPSledDetection[0xBB] = TRUE;
	abNOPSledDetection[0xBC] = TRUE;
	abNOPSledDetection[0xBD] = TRUE;
	abNOPSledDetection[0xBE] = TRUE;
	abNOPSledDetection[0xBF] = TRUE;
	abNOPSledDetection[0xC0] = TRUE;
	abNOPSledDetection[0xC1] = TRUE;
	abNOPSledDetection[0xC2] = FALSE;
	abNOPSledDetection[0xC3] = FALSE;
	abNOPSledDetection[0xC4] = FALSE;
	abNOPSledDetection[0xC5] = FALSE;
	abNOPSledDetection[0xC6] = TRUE;
	abNOPSledDetection[0xC7] = TRUE;
	abNOPSledDetection[0xC8] = FALSE;
	abNOPSledDetection[0xC9] = FALSE;
	abNOPSledDetection[0xCA] = FALSE;
	abNOPSledDetection[0xCB] = FALSE;
	abNOPSledDetection[0xCC] = FALSE;
	abNOPSledDetection[0xCD] = FALSE;
	abNOPSledDetection[0xCE] = FALSE;
	abNOPSledDetection[0xCF] = FALSE;
	abNOPSledDetection[0xD0] = TRUE;
	abNOPSledDetection[0xD1] = TRUE;
	abNOPSledDetection[0xD2] = TRUE;
	abNOPSledDetection[0xD3] = TRUE;
	abNOPSledDetection[0xD4] = TRUE;
	abNOPSledDetection[0xD5] = TRUE;
	abNOPSledDetection[0xD6] = TRUE;
	abNOPSledDetection[0xD7] = FALSE;
	abNOPSledDetection[0xD8] = FALSE;
	abNOPSledDetection[0xD9] = FALSE;
	abNOPSledDetection[0xDA] = FALSE;
	abNOPSledDetection[0xDB] = FALSE;
	abNOPSledDetection[0xDC] = FALSE;
	abNOPSledDetection[0xDD] = FALSE;
	abNOPSledDetection[0xDE] = FALSE;
	abNOPSledDetection[0xDF] = FALSE;
	abNOPSledDetection[0xE0] = FALSE;
	abNOPSledDetection[0xE1] = FALSE;
	abNOPSledDetection[0xE2] = FALSE;
	abNOPSledDetection[0xE3] = FALSE;
	abNOPSledDetection[0xE4] = FALSE;
	abNOPSledDetection[0xE5] = FALSE;
	abNOPSledDetection[0xE6] = FALSE;
	abNOPSledDetection[0xE7] = FALSE;
	abNOPSledDetection[0xE8] = FALSE;
	abNOPSledDetection[0xE9] = FALSE;
	abNOPSledDetection[0xEA] = FALSE;
	abNOPSledDetection[0xEB] = FALSE;
	abNOPSledDetection[0xEC] = FALSE;
	abNOPSledDetection[0xED] = FALSE;
	abNOPSledDetection[0xEE] = FALSE;
	abNOPSledDetection[0xEF] = FALSE;
	abNOPSledDetection[0xF0] = FALSE;
	abNOPSledDetection[0xF1] = FALSE;
	abNOPSledDetection[0xF2] = FALSE;
	abNOPSledDetection[0xF3] = FALSE;
	abNOPSledDetection[0xF4] = FALSE;
	abNOPSledDetection[0xF5] = TRUE;
	abNOPSledDetection[0xF6] = TRUE;
	abNOPSledDetection[0xF7] = TRUE;
	abNOPSledDetection[0xF8] = TRUE;
	abNOPSledDetection[0xF9] = TRUE;
	abNOPSledDetection[0xFA] = TRUE;
	abNOPSledDetection[0xFB] = TRUE;
	abNOPSledDetection[0xFC] = TRUE;
	abNOPSledDetection[0xFD] = TRUE;
	abNOPSledDetection[0xFE] = FALSE;
	abNOPSledDetection[0xFF] = FALSE;
}


/******************************************************************************
 * Search pages for nop sleds
 ******************************************************************************/
BOOL AnalyzeNewPagesForNOPSleds(void)
{
	HANDLE hProcess;
	SYSTEM_INFO sSI;
	LPVOID lpMem;
	MEMORY_BASIC_INFORMATION sMBI;
	static LPVOID alpvPages[MAX_PAGES];
	static int iPages;
	int iIter;
	PBYTE pbPage;
	PBYTE pbStartNOPSled;
	PBYTE pbStartNOPSledMax;
	SIZE_T stCountNOP;
	SIZE_T stCountNOPMax;
	BOOL bFirstRun;
	BYTE bPreviousByte;
	BYTE bOperationLargestSled;
	TCHAR szOutput[256];

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, GetCurrentProcessId());
	if (NULL == hProcess)
	{
		PrintInfo(_TEXT("OpenProcess failed"));
		return TRUE;
	}

	__try
	{
		GetSystemInfo(&sSI);

		bFirstRun = iPages == 0;
		for (lpMem = 0; lpMem < sSI.lpMaximumApplicationAddress; lpMem = (LPVOID)((DWORD)sMBI.BaseAddress + (DWORD)sMBI.RegionSize))
		{
			if (!VirtualQueryEx(hProcess, lpMem, &sMBI, sizeof(MEMORY_BASIC_INFORMATION)))
			{
				PrintError(_TEXT("VirtualQueryEx failed"));
			}
			else
			{
				if (MEM_COMMIT == sMBI.State)
				{
					if (sMBI.Protect == PAGE_READWRITE || sMBI.Protect == PAGE_EXECUTE_READWRITE)
					{
						// Check if we've already scanned this page
						for (iIter = 0; iIter < iPages && iIter < MAX_PAGES; iIter++)
						{
							if (sMBI.BaseAddress == alpvPages[iIter])
							{
								break;
							}
						}
						// TODO SHOULD I don't like this function, as it appears to look for new pages, scan them once,
						// and never scan them again, and also doesn't remove old unused pages from it's array alpvPages,
						// so it could hit the MAX_PAGES before it should, and if it does hit that, then it should probably
						// do something
						if (iIter == iPages && iIter < MAX_PAGES)
						{
							alpvPages[iPages++] = sMBI.BaseAddress;
							if (sHeapLockerSettings.dwVerbose > 0)
								PrintInfo(_TEXT("NOP-sled analysis page 0x%08x protection = 0x%04x size = 0x%04x"), sMBI.BaseAddress, sMBI.Protect, sMBI.RegionSize);
							if (!bFirstRun)
							{
								stCountNOP = 0;
								stCountNOPMax = 0;
								bPreviousByte = *(PBYTE)sMBI.BaseAddress;
								for (pbPage = pbStartNOPSled = (PBYTE)sMBI.BaseAddress; pbPage < (PBYTE)sMBI.BaseAddress + sMBI.RegionSize; pbPage++)
								{
									if (abNOPSledDetection[*pbPage] && bPreviousByte == *pbPage)
									{
										stCountNOP++;
										if (stCountNOP > stCountNOPMax)
										{
											stCountNOPMax = stCountNOP;
											bOperationLargestSled = *pbPage;
											pbStartNOPSledMax = pbStartNOPSled;
										}
									}
									else
									{
										stCountNOP = 0;
										pbStartNOPSled = pbPage;
									}
									bPreviousByte = *pbPage;
								}
								if (stCountNOPMax >= sHeapLockerSettings.dwNOPSledLengthMin)
								{
									PrintInfo(_TEXT(" Size of largest NOP-sled = %ld operation = 0x%02X start = 0x%08X"), stCountNOPMax, bOperationLargestSled, pbStartNOPSledMax);
									if (sHeapLockerSettings.dwForceTermination)
									{
										_sntprintf_s(szOutput, countof(szOutput), _TRUNCATE, _TEXT("This document is probably malicious!\nClick OK to terminate this program (%s).\n\nTechnical details: NOP-sled detected\nlength = %ld\noperation = 0x%02X\nstart = 0x%08X"), NULL2EmptyString(GetExecutableName()), stCountNOPMax, bOperationLargestSled, pbStartNOPSledMax);
									}
									else
									{
										_sntprintf_s(szOutput, countof(szOutput), _TRUNCATE, _TEXT("This document is probably malicious!\nDo you want to terminate this program (%s)?\n\nTechnical details: NOP-sled detected\nlength = %ld\noperation = 0x%02X\nstart = 0x%08X"), NULL2EmptyString(GetExecutableName()), stCountNOPMax, bOperationLargestSled, pbStartNOPSledMax);
									}
									if (FALSE == ThreadedMessageBox(szOutput))
									{
										if (!sHeapLockerSettings.dwResumeMonitoring)
										{
											return FALSE;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	__except(1)
	{
		PrintError("Exception");
	}

	CloseHandle(hProcess);

	return TRUE;
}


/******************************************************************************
 * Search algorithm: http://www-igm.univ-mlv.fr/~lecroq/string/node8.html#SECTION0080
 ******************************************************************************/
void SearchKMPPreCompute(PBYTE pbSearchTerm, int iSearchTermSize, int aiKMPNext[]) {
	int iIter1, iIter2;

	__try
	{
		iIter1 = 0;
		iIter2 = -1;
		aiKMPNext[0] = -1;
		while (iIter1 < iSearchTermSize)
		{
			while (iIter2 > -1 && pbSearchTerm[iIter1] != pbSearchTerm[iIter2])
				iIter2 = aiKMPNext[iIter2];
			iIter1++;
			iIter2++;
			if (pbSearchTerm[iIter1] == pbSearchTerm[iIter2])
				aiKMPNext[iIter1] = aiKMPNext[iIter2];
			else
				aiKMPNext[iIter1] = iIter2;
		}
	}
	__except(1)
	{
		PrintError("Exception");
	}
}


/******************************************************************************
 * 
 ******************************************************************************/
PBYTE SearchPreviousNonWhiteSpaceCharacter(PBYTE pbStart, PBYTE pbLowerLimit, BYTE bCharacter)
{
	__try
	{
		pbStart -= 2;
		while (pbStart >= pbLowerLimit)
		{
			if (*(pbStart + 1) == 0x00 && iswspace(*pbStart))
			{
				pbStart -= 2;
				continue;
			}

			if (*(pbStart + 1) == 0x00 && *pbStart == bCharacter)
			{
				return pbStart;
			}
			else
			{
				return NULL;
			}
		}
	}
	__except(1)
	{
		PrintError("Exception");
	}

	return NULL;
}


/******************************************************************************
 * 
 ******************************************************************************/
PBYTE SearchNextNonWhiteSpaceCharacter(PBYTE pbStart, PBYTE pbUpperLimit, BYTE bCharacter)
{
	__try
	{
		pbStart += 2;
		while (pbStart <= pbUpperLimit)
		{
			if (*(pbStart + 1) == 0x00 && iswspace(*pbStart))
			{
				pbStart += 2;
				continue;
			}

			if (*(pbStart + 1) == 0x00 && *pbStart == bCharacter)
			{
				return pbStart;
			}
			else
			{
				return NULL;
			}
		}
	}
	__except(1)
	{
		PrintError("Exception");
	}

	return NULL;
}


/******************************************************************************
 * Search memory for given string
 ******************************************************************************/
PBYTE SearchFunctionKMP(PBYTE pbSearchTerm, int iSearchTermSize, PBYTE pbMemory, SIZE_T iMemorySize, int iMode)
{
	int iIter1;
	SIZE_T iIter2;
	int aiKMPNext[XSIZE];
	PBYTE pbFound;

	__try
	{
		SearchKMPPreCompute(pbSearchTerm, iSearchTermSize, aiKMPNext);

		iIter1 = 0;
		iIter2 = 0;
		while (iIter2 < iMemorySize)
		{
			while (iIter1 > -1 && pbSearchTerm[iIter1] != pbMemory[iIter2])
			{
				iIter1 = aiKMPNext[iIter1];
			}
			iIter1++;
			iIter2++;
			if (iIter1 >= iSearchTermSize)
			{
				if (0 == iMode)
				{
					return pbMemory + iIter2 - iIter1;
				}
				pbFound = SearchPreviousNonWhiteSpaceCharacter(pbMemory + iIter2 - iIter1, pbMemory, '=');
				if (NULL != pbFound)
				{
					return pbFound;
				}
				pbFound = SearchNextNonWhiteSpaceCharacter(pbMemory + iIter2 - 2, pbMemory + iMemorySize - 1, '(');
				if (NULL != pbFound)
				{
					return pbMemory + iIter2 - iIter1;
				}
				iIter1 = aiKMPNext[iIter1];
			}
		}
	}
	__except(1)
	{
		PrintError("Exception");
	}

	return NULL;
}


/******************************************************************************
 * 
 ******************************************************************************/
BOOL AnalyzeNewPagesToSearchThem(void)
{
	HANDLE hProcess;
	SYSTEM_INFO sSI;
	LPVOID lpMem;
	MEMORY_BASIC_INFORMATION sMBI;
	static LPVOID alpvPages[MAX_PAGES];
	static int iPages;
	int iIter;
	BOOL bFirstRun;
	TCHAR szOutput[256];
	PBYTE pbFound;

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, GetCurrentProcessId());
	if (NULL == hProcess)
	{
		PrintInfo(_TEXT("OpenProcess failed"));
		return TRUE;
	}

	__try
	{
		GetSystemInfo(&sSI);

		bFirstRun = iPages == 0;
		for (lpMem = 0; lpMem < sSI.lpMaximumApplicationAddress; lpMem = (LPVOID)((DWORD)sMBI.BaseAddress + (DWORD)sMBI.RegionSize))
		{
			if (!VirtualQueryEx(hProcess, lpMem, &sMBI, sizeof(MEMORY_BASIC_INFORMATION)))
			{
				PrintError(_TEXT("VirtualQueryEx"));
			}
			else
			{
				if (MEM_COMMIT == sMBI.State)
				{
					if (sMBI.Protect == PAGE_READWRITE || sMBI.Protect == PAGE_EXECUTE_READWRITE)
					{
						for (iIter = 0; iIter < iPages && iIter < MAX_PAGES; iIter++)
						{
							if (sMBI.BaseAddress == alpvPages[iIter])
							{
								break;
							}
						}

						if (iIter == iPages && iIter < MAX_PAGES)
						{
							alpvPages[iPages++] = sMBI.BaseAddress;
							if (sHeapLockerSettings.dwVerbose > 0)
							{
								PrintInfo(_TEXT("Keyword analysis page 0x%08x protection = 0x%04x size = 0x%04x"), sMBI.BaseAddress, sMBI.Protect, sMBI.RegionSize);
							}
							if (!bFirstRun)
							{
								pbFound = SearchFunctionKMP(sHeapLockerSettings.abSearch, sHeapLockerSettings.iSearchLen, (PBYTE)sMBI.BaseAddress, sMBI.RegionSize, sHeapLockerSettings.iSearchMode);
								if (NULL != pbFound)
								{
									PrintInfo(_TEXT(" Found string at 0x%08X"), pbFound);
									if (sHeapLockerSettings.dwForceTermination)
									{
										_sntprintf_s(szOutput, countof(szOutput), _TRUNCATE, _TEXT("This document is probably malicious!\nClick OK to terminate this program (%s).\n\nTechnical details: string detected\nstart = 0x%08X\nstring = %s"), NULL2EmptyString(GetExecutableName()), pbFound, NULL2EmptyString(HexDump(pbFound, 50)));
									}
									else
									{
										_sntprintf_s(szOutput, countof(szOutput), _TRUNCATE, _TEXT("This document is probably malicious!\nDo you want to terminate this program (%s)?\n\nTechnical details: string detected\nstart = 0x%08X\nstring = %s"), NULL2EmptyString(GetExecutableName()), pbFound, NULL2EmptyString(HexDump(pbFound, 50)));
									}

									if (FALSE == ThreadedMessageBox(szOutput))
									{
										if (!sHeapLockerSettings.dwResumeMonitoring)
										{
											return FALSE;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	__except(1)
	{
		PrintError("Exception");
	}

	CloseHandle(hProcess);

	return TRUE;
}


/******************************************************************************
 * Thread to scan new memory for nop sleds
 ******************************************************************************/
DWORD WINAPI MonitorNewPagesToSearchThem(LPVOID lpvArgument)
{
	InitializeabNOPSledDetection();
	while(AnalyzeNewPagesToSearchThem())
	{
		Sleep(1000);
	}

	return 0;
}


/******************************************************************************
 * Allocate page 0 (NULL), to protect against possible problems
 ******************************************************************************/
void PreallocatePage0(void)
{
	// Based on http://www.ivanlef0u.tuxfamily.org/?p=355

	HMODULE hNTDLL;
	NTALLOCATEVIRTUALMEMORY NtAllocateVirtualMemory;
	SIZE_T stRegionSize;
	PVOID pvBaseAddress;
	DWORD dwResult;

	hNTDLL = LoadLibrary(_TEXT("ntdll.dll"));
	if (NULL == hNTDLL)
	{
		if (sHeapLockerSettings.dwVerbose > 0)
			PrintError(_TEXT("LoadLibrary"));
		return;
	}
	else
	{
		NtAllocateVirtualMemory = (NTALLOCATEVIRTUALMEMORY) GetProcAddress(hNTDLL, _TEXT("NtAllocateVirtualMemory"));
		if (NULL == NtAllocateVirtualMemory)
		{
			if (sHeapLockerSettings.dwVerbose > 0)
			{
				PrintError(_TEXT("GetProcAddress failed"));
			}
			return;
		}
		else
		{
			pvBaseAddress = (PVOID) 0x1;
			stRegionSize = 0x1000;
			dwResult = NtAllocateVirtualMemory(GetCurrentProcess(), &pvBaseAddress, 0L, &stRegionSize, MEM_COMMIT | MEM_RESERVE, PAGE_NOACCESS);
			if (sHeapLockerSettings.dwVerbose > 0)
			{
				if (0 == dwResult)
				{
					PrintInfo(_TEXT("NULL page address = %08x memory size = %ld"), pvBaseAddress, stRegionSize);
				}
				else
				{
					PrintInfo(_TEXT("NtAllocateVirtualMemory failed, return code = %ld"), dwResult);
				}
			}
		}
	}
}


/******************************************************************************
 * 
 ******************************************************************************/
DWORD WINAPI HeapLocker(LPVOID lpvArgument)
{
	HKEY hKeyApplication;

	Sleep(100); // Sleep some time to wait for advapi32.dll to load completely (should this DLL be loaded via appinit_dll)

	PrintInfo("Running HeapLocker");

	hKeyApplication = GetApplicationRegKey();

	if (NULL != hKeyApplication)
	{
		ReadHeapLockerSettingsFromRegistryApplication(hKeyApplication);
	}

	ReadHeapLockerSettingsFromRegistry();

	SetHeapLockerSettingsDefaults();

	PrintInfo(_TEXT("Maximum value for PrivateUsage = %ld MB (%d)"), sHeapLockerSettings.dwPrivateUsageMax, sHeapLockerSettings.iOrigin_dwPrivateUsageMax);
	PrintInfo(_TEXT("Minimum value for NOP-sled length = %ld (%d)"), sHeapLockerSettings.dwNOPSledLengthMin, sHeapLockerSettings.iOrigin_dwNOPSledLengthMin);
	PrintInfo(_TEXT("Pre-allocation of generic addresses = %ld (%d)"), sHeapLockerSettings.dwGenericPreAllocate, sHeapLockerSettings.iOrigin_dwGenericPreAllocate);
	PrintInfo(_TEXT("Search mode = %d (%d) length = %d (%d)"), sHeapLockerSettings.iSearchMode, sHeapLockerSettings.iOrigin_iSearchMode, sHeapLockerSettings.iSearchLen, sHeapLockerSettings.iOrigin_iSearchLen);
	PrintInfo(_TEXT("Pre-allocation of NULL page = %ld (%d)"), sHeapLockerSettings.dwPreallocatePage0, sHeapLockerSettings.iOrigin_dwPreallocatePage0);
	PrintInfo(_TEXT("Verbosity = %ld (%d)"), sHeapLockerSettings.dwVerbose, sHeapLockerSettings.iOrigin_dwVerbose);
	PrintInfo(_TEXT("Force process termination = %ld (%d)"), sHeapLockerSettings.dwForceTermination, sHeapLockerSettings.iOrigin_dwForceTermination);
	PrintInfo(_TEXT("Resume monitoring = %ld (%d)"), sHeapLockerSettings.dwResumeMonitoring, sHeapLockerSettings.iOrigin_dwResumeMonitoring);

	ProtectAddresses(hKeyApplication);
	if (NULL != hKeyApplication)
	{
		RegCloseKey(hKeyApplication);
	}

	if (sHeapLockerSettings.dwPreallocatePage0 > 0)
	{
		PreallocatePage0();
	}

	if (sHeapLockerSettings.dwNOPSledLengthMin > 0)
	{
		CreateThread(NULL, 0, MonitorNewPagesForNOPSleds, NULL, 0, NULL);
	}

	if (0xFFFFFFFF != sHeapLockerSettings.dwPrivateUsageMax)
	{
		CreateThread(NULL, 0, MonitorPrivateUsage, NULL, 0, NULL);
	}

	if (sHeapLockerSettings.iSearchLen > 0)
	{
		CreateThread(NULL, 0, MonitorNewPagesToSearchThem, NULL, 0, NULL);
	}

	return 0;
}


/******************************************************************************
 * 
 ******************************************************************************/
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  dwReason,
                       LPVOID lpReserved
					 )
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		PrintInfo("Running OpenHips Protector (ohipsp), compiled on: %s", __TIMESTAMP__);
		PrintInfo("Loaded into a process %d", GetCurrentProcessId());

		CreateThread(NULL, 0, HeapLocker, NULL, 0, NULL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		PrintInfo("Unloading ohipsp from process %d", GetCurrentProcessId());
	}

	return TRUE;
}