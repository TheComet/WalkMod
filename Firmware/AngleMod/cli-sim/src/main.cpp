#include <Windows.h>
#include <thread>
#include "anglemod/uart.h"
#include <xc.h>
#include <cstdio>

void pic16_init(void);
void pic16_process_events(void);

static void DataReceived(const char* buf, DWORD bytesRead)
{
    for (DWORD i = 0; i != bytesRead; ++i)
    {
        RC1REG = buf[i];
        uart_rx_isr();
    }
}

void COMWrite(HANDLE hPort)
{
    static char buf[TX_BUF_SIZE];
    DWORD len = 0;
    while (PIE1bits.TX1IE)
    {
        uart_tx_isr();
        if (PIE1bits.TX1IE == 0)
            break;
        buf[len++] = TX1REG;
    }
    buf[len] = 0;

    DWORD bytesWritten = 0;
    OVERLAPPED ov = { 0 };
    WriteFile(hPort, buf, len, &bytesWritten, &ov);
    GetOverlappedResult(hPort, &ov, &bytesWritten, TRUE);
    printf("%s", buf);
}

int main(int argc, char** argv)
{
    HANDLE hPort = CreateFileA("\\\\.\\COM6", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
    if (hPort == INVALID_HANDLE_VALUE)
        return -1;

    COMMCONFIG cc;
    /*GetCommState(hPort, &cc.dcb);
    cc.dcb.fDtrControl = DTR_CONTROL_DISABLE;
    cc.dcb.fRtsControl = RTS_CONTROL_DISABLE;
    cc.dcb.fOutxCtsFlow = FALSE;
    cc.dcb.fOutxDsrFlow = FALSE;
    SetCommState(hPort, &cc.dcb);*/

    char readBuf[5];
    char writeBuf[1];
    DWORD bytesRead = 0;
    DWORD bytesWritten = 0;
    HANDLE readEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    HANDLE writeEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    OVERLAPPED readOv = { 0 };
    OVERLAPPED writeOv = { 0 };
    readOv.hEvent = readEvent;
    writeOv.hEvent = writeEvent;

    pic16_init();
    while (1)
    {
        bytesRead = 0;
        if (ReadFile(hPort, readBuf, sizeof(readBuf), &bytesRead, &readOv) == FALSE)
        {
            switch (GetLastError())
            {
            case ERROR_IO_PENDING: {
                switch (WaitForSingleObject(readEvent, INFINITE))
                {
                case WAIT_OBJECT_0:
                    if (GetOverlappedResult(hPort, &readOv, &bytesRead, TRUE) == FALSE)
                        goto exitProgram;
                    break;

                case WAIT_ABANDONED:
                case WAIT_TIMEOUT:
                case WAIT_FAILED:
                    goto exitProgram;
                }
            } break;

            default:
                goto exitProgram;
            }
        }

        DataReceived(readBuf, bytesRead);
        pic16_process_events();
        COMWrite(hPort);

        Sleep(1);
    } exitProgram:;

    CloseHandle(hPort);

    return 0;
}
