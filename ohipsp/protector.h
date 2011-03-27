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

#define MESSAGEBOX_TITLE						_TEXT("OpenHIPS HeapLocker")

#define MAX_PAGES 4096

#define ADDRESS_MODE_NOACCESS	0

#define INDEX_ENTRYPOINT		7
#define INDEX_CREATETHREAD		14
#define INDEX_GETCURRENTTHREAD	21
#define INDEX_SUSPENDTHREAD		29

typedef DWORD (WINAPI *NTALLOCATEVIRTUALMEMORY)(HANDLE, PVOID *, ULONG_PTR, PSIZE_T, ULONG, ULONG);

extern BYTE abHeapLockerShellcode[35];

///////////////////////////////////////////////////////////////////////////////
// Prototypes
// dllmain.cpp
LPTSTR NULL2EmptyString(LPTSTR pszString);
LPTSTR GetExecutableName(void);
LPTSTR HexDump(PBYTE pbFound, int iSize);
DWORD WINAPI Protector(LPVOID lpvArgument);

// monitorMemUsage.cpp
DWORD WINAPI MonitorPrivateUsage(LPVOID lpvArgument);

// scanMemory.cpp
DWORD WINAPI MonitorNewPagesToSearchThem(LPVOID lpvArgument);

// ui.cpp
BOOL ThreadedMessageBox(LPTSTR pszOutput);
PBYTE ShellCodeToEntryPoint(void);
