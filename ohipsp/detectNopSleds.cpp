#include "common.h"
#include "protector.h"

///////////////////////////////////////////////////////////////////////////////
// Globals
BYTE abNOPSledDetection[256];

///////////////////////////////////////////////////////////////////////////////
// Local prototypes
BOOL AnalyzeNewPagesForNOPSleds(void);

///////////////////////////////////////////////////////////////////////////////
// Functions

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
 * Thread to search new pages for NOP sleds
 ******************************************************************************/
DWORD WINAPI MonitorNewPagesForNOPSleds(LPVOID lpvArgument)
{
	InitializeabNOPSledDetection();
	while(AnalyzeNewPagesForNOPSleds())
		Sleep(1000);

	return 0;
}