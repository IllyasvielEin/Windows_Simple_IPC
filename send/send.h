#pragma once

#include "resource.h"
extern "C" {
#include "../IPC.h"
}

#define MAX_LOADSTRING 100

#define IDC_EDITCHILD (WM_USER+101)
#define IDC_BUTTON_SEND (WM_USER+102)

#define IDC_OPTION1 (WM_USER+KMES)
#define IDC_OPTION2 (WM_USER+KSOCKET)
#define IDC_OPTION3 (WM_USER+KPIP)
#define IDC_OPTION4 (WM_USER+KMEMORY)

#define SCREEN_WIDTH  (GetSystemMetrics(SM_CXSCREEN))
#define SCREEN_HEIGHT (GetSystemMetrics(SM_CYSCREEN))
#define MAINWINDOW_WIDTH  400
#define MAINWINDOW_HEIGHT 250
#define MAINWINDOW_X  ((SCREEN_WIDTH-MAINWINDOW_WIDTH)/2)
#define MAINWINDOW_Y  ((SCREEN_HEIGHT-MAINWINDOW_HEIGHT)/2)

#define SENDWINDOW_WIDTH (MAINWINDOW_WIDTH-200)
#define SENDWINDOW_HEIGHT (MAINWINDOW_HEIGHT-150)
#define RECVWINDOW_WIDTH (MAINWINDOW_WIDTH-50)
#define RECVWINDOW_HEIGHT (MAINWINDOW_HEIGHT-100)