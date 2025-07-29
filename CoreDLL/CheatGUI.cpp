#include "CheatGUI.h"
#include "MemoryDumper.h"
#include <commctrl.h>
#include <d2d1.h>
#include <wincodec.h>

bool CheatGUI::isVisible = false;
HWND CheatGUI::hWindow = nullptr;
ID2D1Factory* CheatGUI::pFactory = nullptr;
ID2D1HwndRenderTarget* CheatGUI::pRenderTarget = nullptr;
ID2D1Bitmap* CheatGUI::pBackground = nullptr;
IWICImagingFactory* CheatGUI::pWICFactory = nullptr;

void CheatGUI::LoadBackgroundImage(const wchar_t* path) {
    if (!pWICFactory || !pRenderTarget) return;

    IWICBitmapDecoder* pDecoder = nullptr;
    IWICBitmapFrameDecode* pFrame = nullptr;
    IWICFormatConverter* pConverter = nullptr;

    // Создаем декодер для файла
    HRESULT hr = pWICFactory->CreateDecoderFromFilename(
        path,
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &pDecoder
    );

    if (SUCCEEDED(hr)) {
        // Получаем первый кадр изображения
        hr = pDecoder->GetFrame(0, &pFrame);
    }

    if (SUCCEEDED(hr)) {
        // Создаем конвертер формата
        hr = pWICFactory->CreateFormatConverter(&pConverter);
    }

    if (SUCCEEDED(hr)) {
        // Конвертируем в формат, понятный Direct2D
        hr = pConverter->Initialize(
            pFrame,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.0,
            WICBitmapPaletteTypeMedianCut
        );
    }

    if (SUCCEEDED(hr)) {
        // Создаем Direct2D bitmap
        hr = pRenderTarget->CreateBitmapFromWicBitmap(
            pConverter,
            nullptr,
            &pBackground
        );
    }

    // Освобождаем ресурсы
    if (pConverter) pConverter->Release();
    if (pFrame) pFrame->Release();
    if (pDecoder) pDecoder->Release();
}

void CheatGUI::Initialize(HINSTANCE hInstance) {
    // Инициализируем COM
    CoInitialize(nullptr);

    // Создаем фабрику для работы с изображениями
    CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pWICFactory)
    );

    // Создаем фабрику Direct2D
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);

    // Регистрируем класс окна
    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = L"CheatWindow";
    RegisterClassExW(&wc);

    // Создаем окно
    hWindow = CreateWindowExW(
        0,
        L"CheatWindow",
        L"MTA Cheat",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    // Инициализируем рендер-таргет
    ResizeRenderTarget();

    // Создаем кнопку
    CreateWindowW(
        L"BUTTON",
        L"Dump LUAC Files",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        20, 20, 150, 30,
        hWindow,
        (HMENU)1,
        hInstance,
        nullptr
    );

    // Загружаем фоновое изображение
    LoadBackgroundImage(L"C:\\cheat\\background.png");
    ShowWindow(hWindow, SW_HIDE);
}

// Функция для изменения размера рендер-таргета при изменении окна
void CheatGUI::ResizeRenderTarget() {
    if (!hWindow || !pFactory) return;

    // Освобождаем старый рендер-таргет
    if (pRenderTarget) {
        pRenderTarget->Release();
        pRenderTarget = nullptr;
    }

    // Создаем новый рендер-таргет
    RECT rc;
    GetClientRect(hWindow, &rc);
    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

    pFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(hWindow, size),
        &pRenderTarget
    );

    // Перезагружаем фоновое изображение
    if (pBackground) {
        pBackground->Release();
        pBackground = nullptr;
        LoadBackgroundImage(L"C:\\cheat\\background.png");
    }
}

void CheatGUI::Render() {
    if (!isVisible) {
        ShowWindow(hWindow, SW_HIDE);
        return;
    }

    // Если рендер-таргет не создан, создаем его
    if (!pRenderTarget) {
        ResizeRenderTarget();
    }

    pRenderTarget->BeginDraw();
    pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::LightGray));

    // Рисуем фон
    if (pBackground) {
        D2D1_SIZE_F rtSize = pRenderTarget->GetSize();
        D2D1_SIZE_F bgSize = pBackground->GetSize();

        // Масштабируем изображение под размер окна
        D2D1_RECT_F rect = D2D1::RectF(0, 0, rtSize.width, rtSize.height);
        pRenderTarget->DrawBitmap(pBackground, rect);
    }

    pRenderTarget->EndDraw();

    ShowWindow(hWindow, SW_SHOW);
    UpdateWindow(hWindow);
}

void CheatGUI::Toggle() {
    isVisible = !isVisible;
    Render();
}

// Обработчик сообщений окна с поддержкой изменения размера
LRESULT CALLBACK CheatGUI::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND:
        if (LOWORD(wParam) == 1) {
            HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
            if (hProcess) {
                MemoryDumper::DumpLuacFiles(hProcess);
                CloseHandle(hProcess);
            }
        }
        break;

    case WM_SIZE:
        // При изменении размера окна обновляем рендер-таргет
        ResizeRenderTarget();
        InvalidateRect(hWnd, nullptr, FALSE); // Требуем перерисовки
        break;

    case WM_CLOSE:
        ShowWindow(hWnd, SW_HIDE);
        isVisible = false;
        break;

    case WM_DESTROY:
        // Освобождаем ресурсы
        if (pBackground) pBackground->Release();
        if (pRenderTarget) pRenderTarget->Release();
        if (pFactory) pFactory->Release();
        if (pWICFactory) pWICFactory->Release();
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    return 0;
}