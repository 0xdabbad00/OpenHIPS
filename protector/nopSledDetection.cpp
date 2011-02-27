#include "common.h"
#include "protector.h"

DWORD WINAPI MonitorNewPagesForNOPSleds(LPVOID lpvArgument)
{
	InitializeabNOPSledDetection();
	while(AnalyzeNewPagesForNOPSleds())
		Sleep(1000);

	return 0;
}