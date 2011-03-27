#include "common.h"
#include "protector.h"

///////////////////////////////////////////////////////////////////////////////
// Globals

///////////////////////////////////////////////////////////////////////////////
// Local prototypes


///////////////////////////////////////////////////////////////////////////////
// Functions

/******************************************************************************
 * Identifies new memory that has been allocated, and calls the function to
 * scan them.
 ******************************************************************************/
BOOL IdentifyNewMemoryPagesAndScan(void)
{
	// TODO It would probably be better to hook alloc and free to identify new pages and scan
	// slightly after they have been alloc'd (need to know when mem has been copied in)
	// TODO hook VirtualProtect function to identify when memory becomes executable
	HANDLE hProcess;
	SYSTEM_INFO sSI;
	LPVOID lpMem;
	MEMORY_BASIC_INFORMATION sMBI;
	static LPVOID alpvPages[MAX_PAGES];
	static int iPages;
	int iIter;
	BOOL bFirstRun;

	// Open the current process so we can find out the memory used
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
		// Scan through every memory allocation of this process
		// TODO Need to increment differently, because sMBI might not be set if the VirtualQuery call fails
		for (lpMem = 0; lpMem < sSI.lpMaximumApplicationAddress; lpMem = (LPVOID)((DWORD)sMBI.BaseAddress + (DWORD)sMBI.RegionSize))
		{
			// Get info about the memory
			if (!VirtualQueryEx(hProcess, lpMem, &sMBI, sizeof(MEMORY_BASIC_INFORMATION)))
			{
				PrintError(_TEXT("VirtualQueryEx failed for 0x%p protection = 0x%04x size = 0x%04x"), sMBI.BaseAddress, sMBI.Protect, sMBI.RegionSize);
				continue;
			}
			
			// Only scan memory that has been committed
			if (MEM_COMMIT != sMBI.State)
			{
				continue;
			}
			
			// Only scan memory that we can read
			// TODO Set the memory to readable, then back to what it was before
			if (sMBI.Protect != PAGE_READWRITE && sMBI.Protect != PAGE_EXECUTE_READWRITE)
			{
				continue;
			}
				
			// Check if we already scanned this page before
			// TODO Need to go back and clear out pages that are no longer used
			for (iIter = 0; iIter < iPages && iIter < MAX_PAGES; iIter++)
			{
				if (sMBI.BaseAddress == alpvPages[iIter])
				{
					break;
				}
			}

			// Check if this is the newest page we have seen so far, but did not overflow our buffer
			if (iIter == iPages && iIter < MAX_PAGES)
			{
				// Record that we've now seen this page
				alpvPages[iPages++] = sMBI.BaseAddress;
							
				PrintInfo(_TEXT("Found new page 0x%p protection = 0x%04x size = 0x%04x"), sMBI.BaseAddress, sMBI.Protect, sMBI.RegionSize);
							
				// If this is the first run, then do not scan it because we do not want to hit on the memory 
				// containing what signatures to look for, and also don't care to look at the DLL's and .exe's
				if (!bFirstRun)
				{
					// TODO Scan the page
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
 * Thread to scan look for new memory to scan
 ******************************************************************************/
DWORD WINAPI MonitorNewPagesToSearchThem(LPVOID lpvArgument)
{
	while(IdentifyNewMemoryPagesAndScan())
	{
		Sleep(1000);
	}

	return 0;
}