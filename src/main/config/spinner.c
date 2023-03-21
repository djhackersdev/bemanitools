#include <windows.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#define METRIC_COLOR RGB(0, 0, 0)
#define METRIC_RADIUS_CIRCLE 0.8f
#define METRIC_RADIUS_NOTCH 0.2f
#define METRIC_THICKNESS 3

static const wchar_t spinner_cls[] = L"spinner";

static LRESULT CALLBACK
spinner_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
static LRESULT spinner_handle_paint(HWND hwnd);
static LRESULT spinner_handle_update(HWND hwnd, uint8_t pos);

void spinner_init(HINSTANCE inst)
{
    WNDCLASSEX wcx;

    memset(&wcx, 0, sizeof(wcx));
    wcx.cbSize = sizeof(wcx);
    wcx.lpfnWndProc = spinner_wnd_proc;
    wcx.hInstance = inst;
    wcx.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wcx.lpszClassName = spinner_cls;

    RegisterClassEx(&wcx);
}

static LRESULT CALLBACK
spinner_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
        case WM_CREATE:
            SetWindowLong(hwnd, GWLP_USERDATA, 0);

            return TRUE;

        case WM_PAINT:
            return spinner_handle_paint(hwnd);

        case WM_USER:
            return spinner_handle_update(hwnd, (uint8_t) lparam);

        default:
            return DefWindowProc(hwnd, msg, wparam, lparam);
    }
}

static LRESULT spinner_handle_paint(HWND hwnd)
{
    HDC dc;
    HPEN pen;
    PAINTSTRUCT ps;
    RECT r;
    float cx;
    float cy;
    float dx;
    float dy;
    float theta;
    uint8_t itheta;

    /* GWLP_USERDATA angle has 256 steps in a complete revolution.
       To convert to radians, multiply by (2 * PI / 256). */

    itheta = (uint8_t) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    theta = itheta * 0.0245437f;

    GetWindowRect(hwnd, &r);

    cx = (r.right - r.left) / 2.0f;
    cy = (r.bottom - r.top) / 2.0f;

    dc = BeginPaint(hwnd, &ps);

    pen = CreatePen(PS_SOLID, METRIC_THICKNESS, METRIC_COLOR);
    SelectObject(dc, pen);
    SelectObject(dc, GetStockObject(WHITE_BRUSH));

    Ellipse(
        dc,
        (int) (cx * (1.0f - METRIC_RADIUS_CIRCLE)),
        (int) (cy * (1.0f - METRIC_RADIUS_CIRCLE)),
        (int) (cx * (1.0f + METRIC_RADIUS_CIRCLE)),
        (int) (cy * (1.0f + METRIC_RADIUS_CIRCLE)));

    dx = sinf(theta);
    dy = -cosf(theta);

    MoveToEx(
        dc,
        (int) (cx * (1.0f + dx * METRIC_RADIUS_NOTCH)),
        (int) (cy * (1.0f + dy * METRIC_RADIUS_NOTCH)),
        NULL);

    LineTo(
        dc,
        (int) (cx * (1.0f + dx * METRIC_RADIUS_CIRCLE)),
        (int) (cy * (1.0f + dy * METRIC_RADIUS_CIRCLE)));

    DeleteObject(pen);
    EndPaint(hwnd, &ps);

    return 0;
}

static LRESULT spinner_handle_update(HWND hwnd, uint8_t pos)
{
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM) pos);
    InvalidateRect(hwnd, NULL, TRUE);

    return 0;
}

void spinner_fini(HINSTANCE inst)
{
    UnregisterClass(spinner_cls, inst);
}
