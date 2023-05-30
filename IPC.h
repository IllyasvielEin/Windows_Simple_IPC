#pragma once

// 添加要在此处预编译的标头
#include "framework.h"
#include "IPCDefines.h"

static SOCKET ListenSocket;

typedef struct TagMsg {
    char buffer[BUF_SIZE];
    int length;
} TagMsg;

bool SendStr(HWND hWnd, TagMsg* msg, int mode_sel);

bool RecvStr(char* msg);

bool InitIPCAll();

bool RecvStrFromMes(LPARAM lParam, char* msg);

static bool InitSocket();

static SOCKET CreateListenSocket1();

static SOCKET CreateListenSocket2();

static bool RecvStrFromSocket(SOCKET ls, char* msg);