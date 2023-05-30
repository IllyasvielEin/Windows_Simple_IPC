#pragma once

#include "resource.h"
#include "../IPC.h"

#define MAX_LOADSTRING 100

#define IDC_EDITCHILD 101
#define IDC_BUTTON_SEND 102
#define WM_COMMESSAGE (WM_USER+100)

#define SCREEN_WIDTH  (GetSystemMetrics(SM_CXSCREEN))
#define SCREEN_HEIGHT (GetSystemMetrics(SM_CYSCREEN))
#define MAINWINDOW_WIDTH  350
#define MAINWINDOW_HEIGHT 250
#define MAINWINDOW_X  ((SCREEN_WIDTH-MAINWINDOW_WIDTH)/2)
#define MAINWINDOW_Y  ((SCREEN_HEIGHT-MAINWINDOW_HEIGHT)/2)

#define SENDWINDOW_WIDTH (MAINWINDOW_WIDTH-150)
#define SENDWINDOW_HEIGHT (MAINWINDOW_HEIGHT-150)
#define RECVWINDOW_WIDTH (MAINWINDOW_WIDTH-50)
#define RECVWINDOW_HEIGHT (MAINWINDOW_HEIGHT-100)