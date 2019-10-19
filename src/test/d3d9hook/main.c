#include <d3d9.h>
#include <stdio.h>
#include <windows.h>
#include <winuser.h>

#include "test/check.h"

#define debug_print(...) fprintf(stderr, __VA_ARGS__)

static const int render_loop_iterations = 100;

static LRESULT CALLBACK
wnd_proc(HWND wnd, UINT msg, WPARAM w_param, LPARAM l_param);

static HWND create_window()
{
    WNDCLASS wc;
    HRESULT hr;

    memset(&wc, 0, sizeof(WNDCLASS));

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hbrBackground = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpfnWndProc = (WNDPROC) wnd_proc;
    wc.hInstance = NULL;
    wc.lpszClassName = "d3d9hook";

    hr = RegisterClass(&wc);
    check_int_neq(hr, 0);

    return CreateWindow(
        "d3d9hook",
        "d3d9hook test",
        WS_OVERLAPPEDWINDOW,
        0,
        0,
        640,
        480,
        NULL,
        NULL,
        NULL,
        0);
}

static D3DPRESENT_PARAMETERS create_presenter_params(HWND window)
{
    D3DPRESENT_PARAMETERS pp;

    memset(&pp, 0, sizeof(D3DPRESENT_PARAMETERS));

    pp.BackBufferWidth = 640;
    pp.BackBufferHeight = 480;
    pp.BackBufferFormat = D3DFMT_A8R8G8B8;
    pp.BackBufferCount = 1;
    pp.MultiSampleType = D3DMULTISAMPLE_NONE;
    pp.MultiSampleQuality = 0;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.hDeviceWindow = window;
    pp.Windowed = TRUE;
    pp.EnableAutoDepthStencil = TRUE;
    pp.AutoDepthStencilFormat = D3DFMT_D24S8;
    pp.Flags = 0;
    pp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    return pp;
}

static IDirect3DTexture9 *create_texture(IDirect3DDevice9 *dev)
{
    HRESULT hr;
    IDirect3DTexture9 *texture;

    hr = IDirect3DDevice9_CreateTexture(
        dev, 320, 240, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture, NULL);
    check_int_eq(hr, S_OK);

    return texture;
}

int main(int argc, char **argv)
{
    DISPLAY_DEVICEA display_device;
    BOOL res;
    HWND window;
    RECT rect;
    MSG msg;
    IDirect3D9 *d3d9;
    HRESULT hr;
    D3DPRESENT_PARAMETERS pp;
    IDirect3DDevice9 *dev;
    int loop_count;
    IDirect3DTexture9 *texture;

    res = EnumDisplayDevicesA(
        NULL, 0, &display_device, EDD_GET_DEVICE_INTERFACE_NAME);
    check_int_eq(res, TRUE);

    window = create_window();
    check_non_null(window);

    res = GetClientRect(window, &rect);
    check_int_eq(res, TRUE);

    d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
    check_non_null(d3d9);

    pp = create_presenter_params(window);

    hr = IDirect3D9_CreateDevice(
        d3d9,
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        window,
        D3DCREATE_HARDWARE_VERTEXPROCESSING,
        &pp,
        &dev);
    check_int_eq(hr, S_OK);

    IDirect3DDevice9_Release(d3d9);
    d3d9 = NULL;

    texture = create_texture(dev);

    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);

    memset(&msg, 0, sizeof(MSG));

    loop_count = 0;

    // TODO create vertex buffer, upload and render

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            hr = IDirect3DDevice9_Clear(
                dev,
                0,
                NULL,
                D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                0xff0000ff,
                1.0f,
                0);
            check_int_eq(hr, S_OK);

            hr = IDirect3DDevice9_SetRenderState(
                dev, D3DRS_FILLMODE, D3DFILL_POINT);
            check_int_eq(hr, S_OK);

            hr = IDirect3DDevice9_BeginScene(dev);
            check_int_eq(hr, S_OK);

            hr = IDirect3DDevice9_EndScene(dev);
            check_int_eq(hr, S_OK);

            hr = IDirect3DDevice9_Present(dev, NULL, NULL, NULL, NULL);
            check_int_eq(hr, S_OK);

            loop_count++;

            if (loop_count > render_loop_iterations) {
                PostQuitMessage(0);
            }
        }
    }

    IDirect3DTexture9_Release(texture);
    IDirect3DDevice9_Release(dev);

    return 0;
}

LRESULT CALLBACK wnd_proc(HWND wnd, UINT msg, WPARAM w_param, LPARAM l_param)
{
    switch (msg) {
        case WM_CREATE:
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }

    return DefWindowProc(wnd, msg, w_param, l_param);
}