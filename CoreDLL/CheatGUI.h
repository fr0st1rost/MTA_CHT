#pragma once
#include <Windows.h>
#include <d2d1.h>
#include <wincodec.h>  
#pragma comment(lib, "windowscodecs.lib")

class CheatGUI {
public:
    static void Initialize(HINSTANCE hInstance);
   static void ResizeRenderTarget();
    static void Render();
    static void Toggle();
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static void LoadBackgroundImage(const wchar_t* path);

private:
    static bool isVisible;
    static HWND hWindow;
    static ID2D1Factory* pFactory;
    static ID2D1HwndRenderTarget* pRenderTarget;
    static ID2D1Bitmap* pBackground;
    static IWICImagingFactory* pWICFactory;  

};