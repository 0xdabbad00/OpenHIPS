#include "common.h"
#include "protector.h"

///////////////////////////////////////////////////////////////////////////////
// Globals
DWORD dwPrivateUsageMax = 500*1024*1024; // 500MB

///////////////////////////////////////////////////////////////////////////////
// Local prototypes
BOOL CheckPrivateUsage(void);

///////////////////////////////////////////////////////////////////////////////
// Functions

/******************************************************************************
 * Check private memory usage
 ******************************************************************************/
BOOL CheckPrivateUsage(void)
{
	PROCESS_MEMORY_COUNTERS_EX sPMCE;
	TCHAR szOutput[256];

	GetProcessMemoryInfo(GetCurrentProcess(), (PPROCESS_MEMORY_COUNTERS)&sPMCE, sizeof(sPMCE));
	PrintInfo(_TEXT("PrivateUsage %ld MB"), sPMCE.PrivateUsage / 1024 / 1024);
	
	//	PrintInfo(_TEXT("Sum %ld MB"), (sPMCE.PrivateUsage + sPMCE.WorkingSetSize + sPMCE.QuotaPagedPoolUsage + sPMCE.QuotaNonPagedPoolUsage + sPMCE.PagefileUsage) / 1024 / 1024);
	if (sPMCE.PrivateUsage / 1024 / 1024 >= dwPrivateUsageMax)
	{
		// Application is using more than the configured max, display message box
		_sntprintf_s(szOutput, countof(szOutput), _TRUNCATE, _TEXT("This document is probably malicious!\nDo you want to terminate this program (%s)?\n\nTechnical details: heap-spray detected\nPrivateUsage %ld MB"), NULL2EmptyString(GetExecutableName()), sPMCE.PrivateUsage / 1024 / 1024);
		

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