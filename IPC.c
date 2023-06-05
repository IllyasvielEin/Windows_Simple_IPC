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
        SOCKADDR_IN addr;
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

        if (SOCKET_ERROR == connect(ConnectSocket, (SOCKADDR*)&addr, sizeof addr)) {
            closesocket(ConnectSocket);
            WSACleanup();
            return false;
        }

        int iResult;
        iResult = send(ConnectSocket, msg->buffer, msg->length, 0);
        if (iResult == SOCKET_ERROR) {
            WSACleanup();
            return false;
        }
        

        iResult = shutdown(ConnectSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            WSACleanup();
            return false;
        }

        susceed = true;
    }
    case KMEMORY:
    {
        h_file = CreateFileMapping(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            BUF_SIZE,
            L"MyFile");

        if (h_file == NULL) {
            return false;
        }

        pBuf = (LPTSTR)MapViewOfFile(h_file,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            BUF_SIZE);

        if (pBuf == NULL) {
            CloseHandle(h_file);
            return 1;
        }

        strcpy_s((char*)pBuf, BUF_SIZE, (const char*)msg);
        

        
        // 清理资源
        UnmapViewOfFile(pBuf);
        //CloseHandle(h_file);
    }
    case KPIP:
    {

        h_pip = CreateNamedPipe(L"\\\\.\\pipe\\mynamedpipe",
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED | WRITE_DAC,
            PIPE_TYPE_BYTE | PIPE_TYPE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            1024,
            1024,
            NMPWAIT_USE_DEFAULT_WAIT,
            NULL
        );

        if (h_pip == INVALID_HANDLE_VALUE) {
            return false;
        }

        // 连接管道
        if (!ConnectNamedPipe(h_pip, NULL)) {
            CloseHandle(h_pip);
            return false;
        }

        // 发送字符串
        
        DWORD numBytesWritten;
        if (!WriteFile(h_pip, msg, (int)msg->length, &numBytesWritten, NULL)) {
            CloseHandle(h_pip);
            return false;
        }

        // 关闭管道句柄
        //CloseHandle(h_pip);
        return true;
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
        strcpy_s(msg, tmp->length, tmp->buffer);
        msg = tmp->buffer;
        return TRUE;
    }
    return FALSE;
}

bool InitRecvIPCAll()
{
    bool succeed = false;
    if (InitSocket()) {
        succeed = true;
    }

    return succeed;
}

bool InitSendIPCAll()
{
    return false;
}

bool InitSocket()
{
    ListenSocket = CreateListenSocket();
    if (ListenSocket == SOCKET_ERROR) {
        return false;
    }

    unsigned long ul = 1;
    if (ioctlsocket(ListenSocket, FIONBIO, (unsigned long*)&ul)) {
        return false;
    }

    FD_ZERO(&fdSocket);
    FD_SET(ListenSocket, &fdSocket);

    return true;
}

SOCKET CreateListenSocket() {
    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == ListenSocket) {
        WSACleanup();
        return INVALID_SOCKET;
    }

    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.S_un.S_addr = INADDR_ANY;
    addr.sin_port = htons(DEFAULT_PORT_u);
    if (SOCKET_ERROR == bind(ListenSocket, (SOCKADDR*)&addr, sizeof addr)) {
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
    else if (RecvStrFromPip(msg)) {
        sucseed = true;
    }
    else if (RecvStrFromFile(msg)) {
        sucseed = true;
    }
    else {

    }

    return sucseed;
}

bool RecvStrFromSocket(SOCKET ls, char* msg)
{
    if (INVALID_SOCKET == ls) {
        return false;
    }

    if (!msg) return false;

    fd_set fdRead = fdSocket;
    TIMEVAL timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    if (0 >= select(NULL, &fdRead, NULL, NULL, &timeout)) {
        return false;
    }
    for (int i = 0; i < (int)fdSocket.fd_count; ++i) {
        SOCKET s = fdSocket.fd_array[i];
        if (FD_ISSET(s, &fdRead)) {
            if (s == ListenSocket) {
                SOCKET ClientSocket = accept(ls, NULL, NULL);
                if (ClientSocket == INVALID_SOCKET) {
                    continue;
                }
                FD_SET(ClientSocket, &fdSocket);
                return false;
            }
            else {
                char RecvBuffer[BUF_SIZE];
                ZeroMemory(RecvBuffer, BUF_SIZE);
                int iResult = recv(s, RecvBuffer, BUF_SIZE, 0);
                if (iResult > 0) {
                    memcpy(msg, RecvBuffer, iResult);
                }
                else {
                    
                }
                closesocket(s);
                FD_CLR(s, &fdSocket);
            }
        }
    }

    return true;
}

bool RecvStrFromPip(char* msg)
{
    char buffer[BUF_SIZE];

    if (WaitNamedPipe(L"\\\\.\\pipe\\mynamedpipe", NMPWAIT_NOWAIT) == false) {
        return false;
    }

    h_pip = CreateFile(
        L"\\\\.\\pipe\\mynamedpipe",
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (h_pip == INVALID_HANDLE_VALUE) {
        return false;
    }


    DWORD numBytesRead;

    // 从管道读取数据
    if (!ReadFile(h_pip, buffer, BUF_SIZE - 1, &numBytesRead, NULL)) {
        CloseHandle(h_pip);
        return false;
    }
    buffer[numBytesRead] = '\0';

    if (numBytesRead == 0)
    {
        CloseHandle(h_pip);
        return false;
    }

    //msg = new char[numBytesRead + 1];
    strcpy_s(msg, numBytesRead + 1, buffer);

    // 关闭管道句柄
    CloseHandle(h_pip);
    return true;
}

bool RecvStrFromFile(char* msg)
{
    h_file = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        L"MyFile");

    if (h_file == NULL) {
        return false;
    }

    pBuf = (LPTSTR)MapViewOfFile(h_file,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        BUF_SIZE);

    if (pBuf == NULL) {
        CloseHandle(h_file);
        return false;
    }

    //msg = new char[BUF_SIZE];
    strcpy_s(msg, BUF_SIZE, (char*)pBuf);


    // 清理资源
    UnmapViewOfFile(pBuf);
    CloseHandle(h_file);
    return true;
}