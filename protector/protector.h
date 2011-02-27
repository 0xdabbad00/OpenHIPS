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
 * @file		protector\protector.h
 * @summary		Definitions needed by all protector files.
 */

#include "common.h"

#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <psapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

///////////////////////////////////////////////////////////////////////////////
// Defines
#define countof(array) (sizeof(array)/sizeof(array[0]))
#define IS_SUCCESS(x) (ERROR_SUCCESS == (x))
#define FUNCTION_FAILED(x) (!(x))

#define MAX_REGISTRY_KEY_NAME		255
#define MAX_REGISTRY_VALUE_NAME	16383

#define REGISTRY_PATH							_TEXT("Software\\OpenHIPS\\HeapLocker")
#define REGISTRY_ADDRESSES						_TEXT("Addresses")
#define REGISTRY_PATH_APPLICATIONS				_TEXT("Software\\OpenHIPS\\HeapLocker\\Applications")
#define REGISTRY_PRIVATE_USAGE_MAX				_TEXT("PrivateUsageMax")
#define REGISTRY_NOP_SLED_LENGTH_MAX			_TEXT("NOPSledLengthMin")
#define REGISTRY_GENERIC_PRE_ALLOCATE			_TEXT("GenericPreAllocate")
#define REGISTRY_VERBOSE						_TEXT("Verbose")
#define REGISTRY_SEARCH_STRING					_TEXT("SearchString")
#define REGISTRY_SEARCH_MODE					_TEXT("SearchMode")
#define REGISTRY_NULL_PAGE_PRE_ALLOCATE			_TEXT("NullPagePreallocate")
#define REGISTRY_FORCE_TERMINATION				_TEXT("ForceTermination")
#define REGISTRY_RESUME_MONITORING				_TEXT("ResumeMonitoring")

#define MESSAGEBOX_TITLE						_TEXT("HeapLocker")

#define MAX_PAGES 4096

#define NOP 0x90

#define ADDRESS_MODE_NOACCESS	0
#define ADDRESS_MODE_SHELLCODE	1

#define OUTPUTDEBUGSTRING

#define ORIGIN_NONE						0
#define ORIGIN_DEFAULT					1
#define ORIGIN_REGISTRY_GENERIC			2
#define ORIGIN_REGISTRY_APPLICATION		3

typedef struct {
	DWORD dwPrivateUsageMax;
	int iOrigin_dwPrivateUsageMax;
	DWORD dwNOPSledLengthMin;
	int iOrigin_dwNOPSledLengthMin;
	DWORD dwGenericPreAllocate;
	int iOrigin_dwGenericPreAllocate;
	DWORD dwVerbose;
	int iOrigin_dwVerbose;
	BYTE abSearch[1024];
	int iSearchLen;
	int iOrigin_iSearchLen;
	int iSearchMode;
	int iOrigin_iSearchMode;
	DWORD dwPreallocatePage0;
	int iOrigin_dwPreallocatePage0;
	DWORD dwForceTermination;
	int iOrigin_dwForceTermination;
	DWORD dwResumeMonitoring;
	int iOrigin_dwResumeMonitoring;
} HEAPLOCKER_SETTINGS;

#define INDEX_ENTRYPOINT		7
#define INDEX_CREATETHREAD		14
#define INDEX_GETCURRENTTHREAD	21
#define INDEX_SUSPENDTHREAD		29

#define XSIZE 1024

typedef DWORD (WINAPI *NTALLOCATEVIRTUALMEMORY)(HANDLE, PVOID *, ULONG_PTR, PSIZE_T, ULONG, ULONG);

///////////////////////////////////////////////////////////////////////////////
// Prototypes

LPTSTR NULL2EmptyString(LPTSTR pszString);
LPTSTR GetExecutableName(void);
LPTSTR HexDump(PBYTE pbFound, int iSize);
void SuspendThreadsOfCurrentProcessExceptCurrentThread(BOOL bSuspend);
DWORD WINAPI DisplayMessageBox(LPVOID lpvArgument);
BOOL ThreadedMessageBox(LPTSTR pszOutput);
DWORD WINAPI EntryPoint(LPVOID lpvArgument);
PBYTE ShellCodeToEntryPoint(void);
SIZE_T PreallocateAddress(DWORD dwAddress, int iMode);
void ProtectAddresses(HKEY hKeyApplication);
void ReadHeapLockerSettingsFromRegistryApplication(HKEY hKeyApplication);
void ReadHeapLockerSettingsFromRegistry(void);
void SetHeapLockerSettingsDefaults(void);
BOOL CheckPrivateUsage(void);
DWORD WINAPI MonitorPrivateUsage(LPVOID lpvArgument);
HKEY GetApplicationRegKey(void);
void InitializeabNOPSledDetection(void);
BOOL AnalyzeNewPagesForNOPSleds(void);
DWORD WINAPI MonitorNewPagesForNOPSleds(LPVOID lpvArgument);
void SearchKMPPreCompute(PBYTE pbSearchTerm, int iSearchTermSize, int aiKMPNext[]);
PBYTE SearchPreviousNonWhiteSpaceCharacter(PBYTE pbStart, PBYTE pbLowerLimit, BYTE bCharacter);
PBYTE SearchNextNonWhiteSpaceCharacter(PBYTE pbStart, PBYTE pbUpperLimit, BYTE bCharacter);
PBYTE SearchFunctionKMP(PBYTE pbSearchTerm, int iSearchTermSize, PBYTE pbMemory, SIZE_T iMemorySize, int iMode);
BOOL AnalyzeNewPagesToSearchThem(void);
DWORD WINAPI MonitorNewPagesToSearchThem(LPVOID lpvArgument);
void PreallocatePage0(void);
DWORD WINAPI HeapLocker(LPVOID lpvArgument);