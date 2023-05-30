#include "IPC.h"

bool SendStr(HWND hWnd, TagMsg* msg, int mode_sel)
{
    bool susceed = false;
    switch (mode_sel)
    {
    case KMES:
    {
        HWND hWndRecv = NULL;
        do {
            hWndRecv = FindWindow(L"RecverWindow", L"Recver");
        } while (hWndRecv == NULL);
        COPYDATASTRUCT cbs;
        cbs.dwData = 0;
        cbs.cbData = sizeof(TagMsg);
        cbs.lpData = msg;
        if (SendMessage(hWndRecv, WM_COPYDATA, (WPARAM)hWnd, (LPARAM)(LPVOID)&cbs)) {
            susceed = true;
        }
    }
    break;
    case KSOCKET:
    {
        struct addrinfo* result = NULL,
            * ptr = NULL,
            hints;

        ZeroMemory(&hints, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        auto iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
        if (iResult != 0) {
            WSACleanup();
            return false;
        }
        SOCKET ConnectSocket = INVALID_SOCKET;
        ptr = result;
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            WSACleanup();
            return false;
        }

        iResult = send(ConnectSocket, msg->buffer, (int)msg->length, 0);
        if (iResult == SOCKET_ERROR) {
            WSACleanup();
            return false;
        }

        iResult = shutdown(ConnectSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            WSACleanup();
            return false;
        }
    }
    default:
        break;
    }

    if (susceed) {

    }

    return susceed;
}