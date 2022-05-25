#include <Windows.h>
#include <thread>
#include "anglemod/uart.h"
#include <xc.h>
#include <cstdio>

void pic16_init(void);
void pic16_process_events(void);

void COMReadThread(HANDLE hPort)
{
	while (1)
	{
		char c;
		DWORD bytesRead = 0;
		OVERLAPPED ov = {};
		ReadFile(hPort, &c, 1, &bytesRead, &ov);
		GetOverlappedResult(hPort, &ov, &bytesRead, TRUE);
		if (bytesRead == 0) // GetOverlappedResult doesn't block after second read? No idea why
			continue;

		RC1REG = c;
		uart_rx_isr();

		Sleep(10);
	}
}

void COMWriteThread(HANDLE hPort)
{
	while (1) 
	{
		if (PIE1bits.TX1IE)
		{
			uart_tx_isr();
			if (PIE1bits.TX1IE == 0)
				continue;
			char c = TX1REG;

			DWORD bytesWritten = 0;
			OVERLAPPED ov = {};
			WriteFile(hPort, &c, 1, &bytesWritten, &ov);
			GetOverlappedResult(hPort, &ov, &bytesWritten, TRUE);

			Sleep(10);
		}
	}
}

int main(int argc, char** argv)
{
	HANDLE hPort = CreateFile("\\\\.\\COM6", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
	if (hPort == INVALID_HANDLE_VALUE)
		return -1;

	COMMCONFIG cc;
	GetCommState(hPort, &cc.dcb);
	cc.dcb.fDtrControl = DTR_CONTROL_DISABLE;
	cc.dcb.fRtsControl = RTS_CONTROL_DISABLE;
	cc.dcb.fOutxCtsFlow = FALSE;
	cc.dcb.fOutxDsrFlow = FALSE;
	SetCommState(hPort, &cc.dcb);

	std::thread readThread(COMReadThread, hPort);
	std::thread writeThread(COMWriteThread, hPort);

	pic16_init();
	while (1) 
	{
		pic16_process_events();
		Sleep(10);
	}

	readThread.join();
	writeThread.join();

	CloseHandle(hPort);

	return 0;
}
