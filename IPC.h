#pragma once

// ���Ҫ�ڴ˴�Ԥ����ı�ͷ
#include "framework.h"
#include "IPCDefines.h"

typedef struct TagMsg {
    char buffer[BUF_SIZE];
    int length;
} TagMsg;

bool SendStr(HWND hWnd, TagMsg* msg, int mode_sel);

