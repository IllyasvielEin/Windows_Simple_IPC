#pragma once

// 添加要在此处预编译的标头
#include "framework.h"
#include "IPCDefines.h"

#define COMMODE KMES

typedef struct TagMsg {
    char buffer[BUF_SIZE];
    int length;
} TagMsg;

bool SendStr(TagMsg* msg, int mode_sel);

