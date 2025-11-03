// PasteHelper.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "PasteHelper.h"
#include <vector>
#include <string>


#define MAX_LOADSTRING 100
#define ID_EDIT_INPUT 1001
#define ID_BUTTON_ABOUT 1002
#define ID_BUTTON_PASTE 1003

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HWND hEdit = NULL;                              // 输入框句柄
std::wstring g_inputText;                       // 存储输入文本

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                TypeTextWithDelay(const std::wstring& text);

// 函数声明：告诉编译器 OpenNewWindow 是这样的一个函数
void OpenNewWindow(HINSTANCE hInstance);

// 处理滚轮消息并转发给编辑框
LRESULT ForwardWheelMessage(HWND hwndEdit, WPARAM wParam, LPARAM lParam) {
    // 将WM_MOUSEWHEEL消息转发给编辑框
    return SendMessage(hwndEdit, WM_MOUSEWHEEL, wParam, lParam);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PASTEHELPER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PASTEHELPER));

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

    return (int)msg.wParam;
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

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PASTEHELPER));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PASTEHELPER);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}





//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 将实例句柄存储在全局变量中

    // 修改窗口样式，移除WS_THICKFRAME和WS_MAXIMIZEBOX来禁止缩放
    HWND hWnd = CreateWindowW(szWindowClass, szTitle,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, 0, 600, 400, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

// 创建控件
void CreateControls(HWND hWnd) {
    // 创建多行编辑框 - 确保有垂直滚动条
    hEdit = CreateWindowW(L"EDIT", L"",
        WS_BORDER | WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | ES_AUTOHSCROLL | WS_VSCROLL,
        30, 30, 540, 200, // 初始位置和大小
        hWnd, (HMENU)ID_EDIT_INPUT, hInst, nullptr);

    // 设置编辑框字体
    HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"微软雅黑");
    SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 创建关于按钮
    CreateWindowW(L"BUTTON", L"about Duo",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        200, 250, 100, 40, hWnd, (HMENU)ID_BUTTON_ABOUT, hInst, nullptr);

    // 创建粘贴按钮
    CreateWindowW(L"BUTTON", L"paste >",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        320, 250, 100, 40, hWnd, (HMENU)ID_BUTTON_PASTE, hInst, nullptr);
}





//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        CreateControls(hWnd);
        break;

    case WM_SIZE:
    {
        // 获取窗口客户区大小
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);

        // 计算编辑框位置和大小（90%宽度，10行高度）
        int editWidth = rcClient.right * 0.9;
        int editHeight = 200; // 10行高度
        int editX = (rcClient.right - editWidth) / 2;
        int editY = (rcClient.bottom - editHeight - 60) / 2; // 下方留出按钮空间

        // 调整编辑框位置
        SetWindowPos(hEdit, NULL, editX, editY, editWidth, editHeight, SWP_NOZORDER);

        // 调整按钮位置 - 重新计算按钮位置以保持居中且有间距
        int btnY = editY + editHeight + 20;
        int btnWidth = 100;
        int btnHeight = 40;
        int totalBtnWidth = btnWidth * 2 + 20; // 两个按钮宽度 + 20像素间距
        int startBtnX = (rcClient.right - totalBtnWidth) / 2; // 从中间开始

        SetWindowPos(GetDlgItem(hWnd, ID_BUTTON_ABOUT), NULL,
            startBtnX, btnY, btnWidth, btnHeight, SWP_NOZORDER);
        SetWindowPos(GetDlgItem(hWnd, ID_BUTTON_PASTE), NULL,
            startBtnX + btnWidth + 20, btnY, btnWidth, btnHeight, SWP_NOZORDER);
    }
    break;

    // 处理鼠标滚轮消息，将其转发给编辑框
    case WM_MOUSEWHEEL:
        if (hEdit && IsWindowVisible(hEdit) && IsWindowEnabled(hEdit)) {
            // 将滚轮消息转发给编辑框
            SendMessage(hEdit, WM_MOUSEWHEEL, wParam, lParam);
            return 0; // 已处理消息
        }
        break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 分析菜单选择:
        switch (wmId)
        {
        case ID_BUTTON_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;

        case ID_BUTTON_PASTE:
            // 获取编辑框文本
            wchar_t buffer[4096];
            GetDlgItemTextW(hWnd, ID_EDIT_INPUT, buffer, 4096);
            g_inputText = buffer;

            // 设置3秒后开始输入的定时器
            SetTimer(hWnd, 1, 3000, NULL);
            break;

        case IDM_XS:
            system("taskkill /f /im REDAgent.exe");
            break;

        case IDM_XJ:
            system("taskkill / f / im StudentMain.exe");
            break;

        case IDM_TestNet:
            system("ping www.baidu.com");
            break;

        case IDM_CMD:
            system("cmd");
            break;

            // ✅ 新增以下 case，处理 "打开新窗口"
        case IDM_Dino:
            OpenNewWindow(hInst);  // 调用一个函数来打开新窗口
            break;

        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_TIMER:
        if (wParam == 1) {
            KillTimer(hWnd, 1);
            // 开始模拟输入
            TypeTextWithDelay(g_inputText);
        }
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // 绘制背景
        Rectangle(hdc, 0, 0, 600, 400);
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

// 模拟输入文本（每个字符间隔50ms）
void TypeTextWithDelay(const std::wstring& text) {
    if (text.empty()) return;

    for (size_t i = 0; i < text.length(); i++) {
        wchar_t ch = text[i];  // 这里的 ch 可以是中文字符

        // 发送Unicode字符输入 - 这是支持中文的关键
        INPUT input = { 0 };
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = 0; // 不使用虚拟键码
        input.ki.wScan = ch; // 直接发送Unicode字符代码
        input.ki.dwFlags = KEYEVENTF_UNICODE; // 这是支持中文的关键标志
        SendInput(1, &input, sizeof(INPUT));

        // 发送按键释放
        input.ki.dwFlags |= KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));

        // 等待50ms
        Sleep(50);
    }
}

// "关于"框的消息处理程序。
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












