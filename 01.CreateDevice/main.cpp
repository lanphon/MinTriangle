#include <Windows.h>
#include "D3DManager.h"


auto szTitle = L"MinTriangle";
auto szWindowClass = L"MinTriangle";
auto szShaderFile = L"../MinTriangle/MinTriangle.fx";


static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_ERASEBKGND:
            return 0;

        case WM_SIZE:
            D3DManager::GetInstance().Resize(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hWnd, &ps);
                EndPaint(hWnd, &ps);
            }
            return 0;

        case WM_DESTROY:
			D3DManager::GetInstance().Destroy();
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}


int WINAPI WinMain(
        HINSTANCE hInstance, // 現在のインスタンスのハンドル
        HINSTANCE hPrevInstance, // 以前のインスタンスのハンドル
        LPSTR lpCmdLine, // コマンドライン
        int nCmdShow // 表示状態
        )
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // register window class
    {
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = NULL;
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = NULL;//MAKEINTRESOURCE(IDC_D3D11SAMPLE);
        wcex.lpszClassName = szWindowClass;
        wcex.hIconSm = NULL;
        RegisterClassEx(&wcex);
    }

    // create window
    HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, nullptr);
    if (!hWnd) return 1;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // create d3d
	if (!D3DManager::GetInstance().CreateDevice())return 2;

    // main loop
    MSG msg;
    while (true)
    {
        if (PeekMessage (&msg,NULL,0,0,PM_NOREMOVE))
        {
            if (!GetMessage (&msg,NULL,0,0))
                return msg.wParam ;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            // clear render target
            D3DManager::GetInstance().Render(hWnd);
        }
    }
    return (int) msg.wParam;
}
