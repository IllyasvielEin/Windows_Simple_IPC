#pragma once

// ���Ҫ�ڴ˴�Ԥ����ı�ͷ
#include "framework.h"
#include "IPCDefines.h"

#define COMMODE KMES

typedef struct TagMsg {
    char buffer[BUF_SIZE];
    int length;
} TagMsg;

bool SendStr(TagMsg* msg, int mode_sel);

