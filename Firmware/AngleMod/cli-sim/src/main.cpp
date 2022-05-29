#include <Windows.h>
#include <thread>
#include "anglemod/uart.h"
#include "anglemod/joy.h"
#include "anglemod/btn.h"
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
    if (len > 0)
    {
        WriteFile(hPort, buf, len, &bytesWritten, &ov);
        GetOverlappedResult(hPort, &ov, &bytesWritten, TRUE);
        printf("%s", buf);
    }
}

void JoystickToADC(void)
{
    JOYINFO ji;
    joyGetPos(0, &ji);

    joy_tim0_isr();  /* Triggers measurement of X and Y */
    ADRESH = ji.wXpos / 256;
    joy_adc_isr();
    ADRESH = 255 - ji.wYpos / 256;
    joy_adc_isr();

    static UINT lastButtons = 0;
    if (ji.wButtons & ~lastButtons)
    {
        PORTA = 0x10;
        btn_ioc_isr();
    }
    else if (~ji.wButtons & lastButtons)
    {
        PORTA = 0x00;
        btn_ioc_isr();
    }
    lastButtons = ji.wButtons;

}

int main(int argc, char** argv)
{
    HANDLE hPort = CreateFileA("\\\\.\\COM6", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
    if (hPort == INVALID_HANDLE_VALUE)
        return -1;

    char readBuf[5];
    DWORD bytesRead = 0;
    HANDLE readEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    OVERLAPPED readOv = { 0 };
    readOv.hEvent = readEvent;

    bool readPending = false;

    pic16_init();
    while (1)
    {
        if (readPending == false)
        {
            bytesRead = 0;
            if (ReadFile(hPort, readBuf, sizeof(readBuf), &bytesRead, &readOv) == TRUE)
            {
                DataReceived(readBuf, bytesRead);
            }
            else
            {
                switch (GetLastError())
                {
                case ERROR_IO_PENDING:
                    readPending = true;
                    break;
                default:
                    goto exitProgram;
                }
            }
        }

        if (readPending)
        {
            if (GetOverlappedResult(hPort, &readOv, &bytesRead, FALSE))
            {
                DataReceived(readBuf, bytesRead);
                readPending = false;
            }
            else
            {
                if (GetLastError() != ERROR_IO_INCOMPLETE)
                    goto exitProgram;
            }
        }

        JoystickToADC();
        pic16_process_events();
        COMWrite(hPort);

        Sleep(1);
    } exitProgram:;

    CloseHandle(hPort);

    return 0;
}
