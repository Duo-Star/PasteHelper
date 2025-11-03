// Dino.cpp

#include <windows.h>
#include <string>

// 全局窗口类名
const wchar_t CLASS_NAME_NEW_WINDOW[] = L"NewWindowClass";

// ==================== 游戏状态变量（简单全局变量，后续可封装）====================
static bool g_bGameRunning = false;       // 游戏是否运行中
static int g_nDinoY = 200;                // 恐龙 Y 坐标（示例值，您自己调整）
static bool g_bDinoJumping = false;       // 恐龙是否在跳跃
static int g_nDinoVelocityY = 0;          // 恐龙 Y 方向速度
static const int GRAVITY = 2;             // 重力
static const int JUMP_STRENGTH = -15;     // 跳跃初始速度（负值向上）
static const int GROUND_Y = 250;          // 地面 Y 坐标（示例）

// ==================== 窗口过程：游戏主逻辑在这里 ====================
LRESULT CALLBACK NewWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        // 启动游戏循环：每 16ms (~60FPS) 触发一次 WM_TIMER
        SetTimer(hwnd, 1, 16, NULL);
        g_bGameRunning = true;

        // 可选：设置窗口用户数据或其它初始化
    }
    break;

    case WM_TIMER:
        if (wParam == 1 && g_bGameRunning)
        {
            // --------------------------
            // TODO: 在这里更新游戏逻辑
            // 比如：
            // - 恐龙物理（跳跃、重力）
            // - 障碍物移动
            // - 碰撞检测
            // - 得分更新
            // --------------------------

            // 示例：简单的恐龙跳跃物理
            if (g_bDinoJumping)
            {
                g_nDinoVelocityY += GRAVITY;          // 重力加速度
                g_nDinoY += g_nDinoVelocityY;         // 更新 Y 位置

                // 检查是否落地
                if (g_nDinoY >= GROUND_Y)
                {
                    g_nDinoY = GROUND_Y;
                    g_bDinoJumping = false;
                    g_nDinoVelocityY = 0;
                }
            }

            // 触发重绘
            InvalidateRect(hwnd, NULL, FALSE);
        }
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // =========================
        // TODO: 在这里绘制游戏内容（Canvas 区域）
        // 比如：
        // - 背景
        // - 恐龙（矩形）
        // - 障碍物
        // - 地面
        // - 文字 / 得分
        // =========================

        // 【示例绘图：背景】
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        FillRect(hdc, &clientRect, (HBRUSH)(COLOR_WINDOW + 1));  // 浅色背景

        // 【示例绘图：地面】
        RECT groundRect = { 0, GROUND_Y + 20, clientRect.right, clientRect.bottom };
        FillRect(hdc, &groundRect, (HBRUSH)RGB(34, 139, 34));  // 森林绿

        // 【示例绘图：恐龙（简单矩形）】
        RECT dinoRect = { 50, g_nDinoY, 100, g_nDinoY + 40 };  // x, y, x2, y2
        FillRect(hdc, &dinoRect, (HBRUSH)RGB(255, 0, 0));      // 红色恐龙

        // 【示例文字：提示】
        DrawTextW(hdc, L"Google Dino 游戏框架已搭建！按空格跳跃", -1, &clientRect,
            DT_CENTER | DT_TOP | DT_WORDBREAK);

        EndPaint(hwnd, &ps);
    }
    break;

    case WM_KEYDOWN:
        if (wParam == VK_SPACE && !g_bDinoJumping && g_bGameRunning)
        {
            // 恐龙跳跃
            g_bDinoJumping = true;
            g_nDinoVelocityY = JUMP_STRENGTH;
        }
        break;

    case WM_DESTROY:
        // 停止定时器
        KillTimer(hwnd, 1);
        g_bGameRunning = false;
        // 不要调用 PostQuitMessage(0) ，因为我们不希望关闭整个程序
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// ==================== 打开新窗口的函数 ====================
void OpenNewWindow(HINSTANCE hInstance)
{
    WNDCLASSW wc = { 0 };

    wc.lpfnWndProc = NewWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME_NEW_WINDOW;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassW(&wc))
    {
        MessageBoxW(NULL, L"新窗口类注册失败！", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    HWND hNewWnd = CreateWindowW(
        CLASS_NAME_NEW_WINDOW,
        L"?? Google Dino 游戏窗口",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 400,
        NULL, NULL, hInstance, NULL);

    if (hNewWnd == NULL)
    {
        MessageBoxW(NULL, L"新窗口创建失败！", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    ShowWindow(hNewWnd, SW_SHOW);
    UpdateWindow(hNewWnd);
}