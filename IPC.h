#pragma once

// 添加要在此处预编译的标头
#include "framework.h"
#include "IPCDefines.h"

typedef struct TagMsg {
    char buffer[BUF_SIZE];
    int length;
} TagMsg;

bool SendStr(HWND hWnd, TagMsg* msg, int mode_sel);

