#include "common.h"
#include "protector.h"

///////////////////////////////////////////////////////////////////////////////
// Globals

///////////////////////////////////////////////////////////////////////////////
// Local prototypes
void SearchKMPPreCompute(PBYTE pbSearchTerm, int iSearchTermSize, int aiKMPNext[]);
PBYTE SearchPreviousNonWhiteSpaceCharacter(PBYTE pbStart, PBYTE pbLowerLimit, BYTE bCharacter);
PBYTE SearchNextNonWhiteSpaceCharacter(PBYTE pbStart, PBYTE pbUpperLimit, BYTE bCharacter);
PBYTE SearchFunctionKMP(PBYTE pbSearchTerm, int iSearchTermSize, PBYTE pbMemory, SIZE_T iMemorySize, int iMode);
BOOL AnalyzeNewPagesToSearchThem(void);

///////////////////////////////////////////////////////////////////////////////
// Functions

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