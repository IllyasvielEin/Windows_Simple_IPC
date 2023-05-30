// send.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "send.h"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

HWND hWndThis;
HWND hwndButtonOK;
HWND hWndEdit;

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        return FALSE;
    }

    // 初始化全局字符串
    /*LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SEND, szWindowClass, MAX_LOADSTRING);*/
    lstrcpyW(szTitle, L"Sender");
    lstrcpyW(szWindowClass, L"SenderWindow");
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SEND));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SEND));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SEND);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWndThis = CreateWindowW(
       szWindowClass,
       szTitle,
       WS_OVERLAPPEDWINDOW,
       MAINWINDOW_X,
       MAINWINDOW_Y,
       MAINWINDOW_WIDTH,
       MAINWINDOW_HEIGHT,
       nullptr,
       nullptr,
       hInstance,
       nullptr
   );

   if (!hWndThis)
   {
      return FALSE;
   }

   ShowWindow(hWndThis, nCmdShow);
   UpdateWindow(hWndThis);

   // Send
   hWndEdit = CreateWindowEx(
       0, L"EDIT",
       NULL,
       WS_CHILD | WS_VISIBLE | WS_VSCROLL |
       ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL,
       10, 100,
       10, 100,
       hWndThis,
       (HMENU)IDC_EDITCHILD,
       hInst,
       NULL
   );

   ShowWindow(hWndEdit, nCmdShow);
   MoveWindow(hWndEdit, 0, 0, SENDWINDOW_WIDTH, SENDWINDOW_HEIGHT, TRUE);
   UpdateWindow(hWndEdit);

   hwndButtonOK = CreateWindow(
       L"BUTTON",        // predefined class
       L"Send",        // button text
       WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,  // styles
       SENDWINDOW_WIDTH + 10,         // starting x position
       0,         // starting y position
       75,        // button width
       25,        // button height
       hWndThis,     // parent window
       (HMENU)IDC_BUTTON_SEND,       // ID for the OK button
       (HINSTANCE)GetWindowLong(hWndThis, GWL_HINSTANCE),
       NULL
   );      // pointer not needed

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDC_BUTTON_SEND: {
                int length = GetWindowTextLength(hWndEdit);
                if (length >= BUF_SIZE) {
                    MessageBox(hWnd, L"String too big", L"Send Error", MB_OK);
                    break;
                }
                TagMsg msg;
                ZeroMemory(msg.buffer, BUF_SIZE);
                static LPWSTR lpchar = new WCHAR[BUF_SIZE];
                ZeroMemory(lpchar, BUF_SIZE);
                GetWindowText(hWndEdit, lpchar, length+1);
                WideCharToMultiByte(CP_ACP, NULL, lpchar, -1, msg.buffer, length + 1, NULL, FALSE);
                msg.length = length+1;
                if (SendStr(hWnd, &msg, COMMODE)) {
                    MessageBox(hWnd, (LPCWSTR)msg.buffer, L"SendBox", MB_OK);
                }
            } break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
