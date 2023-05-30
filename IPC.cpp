#include "IPC.h"
#include <stdlib.h>

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
        sockaddr_in addr;
        ZeroMemory(&addr, sizeof addr);
        addr.sin_family = AF_INET;
        if (0 == inet_pton(AF_INET, (PCSTR)"127.0.0.1", &addr.sin_addr.S_un.S_addr)) {
            WSACleanup();
            return false;
        }
        addr.sin_port = htons(DEFAULT_PORT_u);


        SOCKET ConnectSocket = INVALID_SOCKET;
        ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (ConnectSocket == INVALID_SOCKET) {
            WSACleanup();
            return false;
        }

        if (SOCKET_ERROR == connect(ConnectSocket, (sockaddr*)&addr, sizeof addr)) {
            closesocket(ConnectSocket);
            WSACleanup();
            return false;
        }

        int iResult;
        do {
            iResult = send(ConnectSocket, msg->buffer, (int)msg->length, 0);
            if (iResult == SOCKET_ERROR) {
                WSACleanup();
                return false;
            }
        } while (iResult <= 0);
        

        iResult = shutdown(ConnectSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            WSACleanup();
            return false;
        }

        susceed = true;
    }
    default:
        break;
    }

    if (susceed) {

    }

    return susceed;
}

bool RecvStrFromMes(LPARAM lParam, char* msg) {
    PCOPYDATASTRUCT pcbs = (PCOPYDATASTRUCT)lParam;
    if (pcbs->dwData == 0) {
        TagMsg* tmp = (TagMsg*)(pcbs->lpData);
        msg = tmp->buffer;
        return TRUE;
    }
    return FALSE;
}

bool InitIPCAll()
{
    bool succeed = false;
    if (InitSocket()) {
        succeed = true;
    }

    return succeed;
}

bool InitSocket()
{
    ListenSocket = CreateListenSocket2();
    if (ListenSocket == SOCKET_ERROR) {
        return false;
    }

    unsigned long ul = 1;
    if (ioctlsocket(ListenSocket, FIONBIO, (unsigned long*)&ul)) {
        return false;
    }

    return true;
}

SOCKET CreateListenSocket1()
{
    struct addrinfo* result = NULL, * ptr = NULL, hints;

    ZeroMemory(&hints, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    int iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        WSACleanup();
        return INVALID_SOCKET;
    }

    ListenSocket = INVALID_SOCKET;
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (ListenSocket == INVALID_SOCKET) {
        freeaddrinfo(result);
        WSACleanup();
        return INVALID_SOCKET;
    }

    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return INVALID_SOCKET;
    }

    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(ListenSocket);
        WSACleanup();
        return INVALID_SOCKET;
    }

    return ListenSocket;
}

SOCKET CreateListenSocket2() {
    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == ListenSocket) {
        WSACleanup();
        return INVALID_SOCKET;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.S_un.S_addr = INADDR_ANY;
    addr.sin_port = htons(DEFAULT_PORT_u);
    if (SOCKET_ERROR == bind(ListenSocket, (sockaddr*)&addr, sizeof addr)) {
        closesocket(ListenSocket);
        WSACleanup();
        return INVALID_SOCKET;
    }

    if (SOCKET_ERROR == listen(ListenSocket, SOMAXCONN)) {
        closesocket(ListenSocket);
        WSACleanup();
        return INVALID_SOCKET;
    }

    return ListenSocket;
}

bool RecvStr(char* msg)
{
    bool sucseed = false;

    if (RecvStrFromSocket(ListenSocket, msg)) {
        sucseed = true;
    }

    return sucseed;
}

bool RecvStrFromSocket(SOCKET ls, char* msg)
{
    if (INVALID_SOCKET == ls) {
        return false;
    }

    SOCKET ClientSocket = accept(ls, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        return false;
    }

    char RecvBuffer[BUF_SIZE];
    ZeroMemory(RecvBuffer, BUF_SIZE);
    int iResult;
    iResult = recv(ClientSocket, RecvBuffer, BUF_SIZE, 0);
    if (iResult == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) {
            closesocket(ClientSocket);
            WSACleanup();
            return false;
        }
    }
    if (iResult > 0) {
        if (!msg) return false;
        memcpy(msg, RecvBuffer, iResult);
        closesocket(ClientSocket);
    }

    return true;
}
