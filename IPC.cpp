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
    case KMEMORY:
    {
        h_file = CreateFileMapping(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            BUF_SIZE,
            L"MyPage");

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

        char input[BUF_SIZE];

        strcpy((char*)pBuf, (const char*)msg);

        // 清理资源
        UnmapViewOfFile(pBuf);
        CloseHandle(h_file);
    }
    case KPIP:
    {
        h_pip = CreateNamedPipe(
            L"MyPipe",
            PIPE_ACCESS_OUTBOUND,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            1,
            BUF_SIZE,
            BUF_SIZE,
            0,
            NULL);
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
        CloseHandle(h_pip);
        return true;
    }
    default:
        break;
    }

    if (susceed) {

    }

    return susceed;
}

bool RecvStrFromMes(LPARAM lParam, char** msg) {
    PCOPYDATASTRUCT pcbs = (PCOPYDATASTRUCT)lParam;
    if (pcbs->dwData == 0) {
        TagMsg* tmp = (TagMsg*)(pcbs->lpData);
        *msg = tmp->buffer;
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
    else if (pass) {

    }
    else if (RecvStrFromPip(msg)) {

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

    SOCKET ClientSocket = accept(ls, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        return false;
    }

    char RecvBuffer[BUF_SIZE];
    int iResult;

    iResult = recv(ClientSocket, RecvBuffer, BUF_SIZE, 0);
    if (iResult > 0) {
        msg = new char[iResult];
        strcpy_s(msg, iResult, RecvBuffer);
        return true;
    }
    else if (iResult == 0) {
        // close
    }
    else {
        closesocket(ClientSocket);
        WSACleanup();
        //return false;
    }

    return true;

    //static bool init = false;
    //static fd_set readfds;
    //if (!init) {
    //    FD_ZERO(&readfds);
    //    FD_SET(ls, &readfds);
    //    init = true;
    //}

    //fd_set temp_fds = readfds;

    //timeval timeout{ .tv_sec = 0, .tv_usec = 0 };
    //if (select(0, &readfds, NULL, NULL, &timeout) == SOCKET_ERROR) {
    //    closesocket(ls);
    //    WSACleanup();
    //    return false;
    //}

    //for (int i = 0; i < temp_fds.fd_count; ++i) {
    //    SOCKET s = temp_fds.fd_array[i];
    //    if (s == ls) {
    //        SOCKET ClientSocket = accept(ls, NULL, NULL);
    //        if (ClientSocket == INVALID_SOCKET) {
    //            continue;
    //        }
    //        FD_SET(ClientSocket, &readfds);
    //    }
    //    else {
    //        char RecvBuffer[BUF_SIZE];
    //        int iResult;
    //        int RecvLen = BUF_SIZE;

    //        iResult = recv(s, RecvBuffer, RecvLen, 0);
    //        if (iResult > 0) {
    //            msg = new char[iResult];
    //            strcpy_s(msg, iResult, RecvBuffer);
    //            return true;
    //        }
    //        else if (iResult == 0) {
    //            // close
    //        }
    //        else {
    //            closesocket(s);
    //            FD_CLR(s, &readfds);
    //            WSACleanup();
    //            //return false;
    //        }
    //    }
    //}

    return false;
}

bool RecvStrFromPip(char* msg)
{
    char buffer[BUF_SIZE];

    h_pip = CreateFile(
        L"MyPipe",
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
    msg = new char[numBytesRead + 1];
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

    msg = new char[BUF_SIZE];
    strcpy(msg, (char*)pBuf);


    // 清理资源
    UnmapViewOfFile(pBuf);
    CloseHandle(h_file);
    return true;
}