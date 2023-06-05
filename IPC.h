#pragma once

// 添加要在此处预编译的标头
#include "framework.h"
#include "IPCDefines.h"
#include <stdbool.h>

static SOCKET ListenSocket;
static fd_set fdSocket;

static HANDLE h_pip;
static HANDLE h_file;
static LPCTSTR pBuf;

typedef struct TagMsg {
    char buffer[BUF_SIZE];
    int length;
} TagMsg;

bool SendStr(HWND hWnd, TagMsg* msg, int mode_sel);

bool RecvStr(char* msg);

bool InitRecvIPCAll();

bool InitSendIPCAll();

bool RecvStrFromMes(LPARAM lParam, char* msg);

static bool InitSocket();

static SOCKET CreateListenSocket1();

static SOCKET CreateListenSocket();

static bool RecvStrFromSocket(SOCKET ls, char* msg);

static bool RecvStrFromPip(char* msg);

static bool RecvStrFromFile(char* msg);