#if defined(AP_OPENGL) && defined(_WIN32)

#include "gfx/gfx.h"
#include "opengl.h"

// See https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_create_context.txt for all values
#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001

#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_COLOR_BITS_ARB                        0x2014
#define WGL_DEPTH_BITS_ARB                        0x2022
#define WGL_STENCIL_BITS_ARB                      0x2023

#define WGL_FULL_ACCELERATION_ARB                 0x2027
#define WGL_TYPE_RGBA_ARB                         0x202B

#define X(ret, name, args) gl_func_##name name = NULL;
    #include "opengl_xlist.h"
#undef X

// TODO: Use os_lib_load

// https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions#Windows_2
static void* wgl_get_proc_address(const char* name) {
    void* p = (void*)wglGetProcAddress(name);
    if (p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1)) {
        HMODULE module = LoadLibrary(L"opengl32.dll");
        ASSERT(module != 0, "Failed to load opengl32.dll");
        p = (void*)GetProcAddress(module, name);
    }
    return p;
}

static LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef HGLRC WINAPI wglCreateContextAttribsARB_func(HDC hdc, HGLRC hShareContext, const int *attribList);
typedef BOOL WINAPI wglChoosePixelFormatARB_func(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);

static wglCreateContextAttribsARB_func *wglCreateContexAttribsARB = NULL;
static wglChoosePixelFormatARB_func    *wglChoosePixelFormatARB   = NULL;

gfx_window* gfx_win_create(marena* arena, u32 width, u32 height, string8 title) {
    gfx_window* win = CREATE_ZERO_STRUCT(arena, gfx_window);

    win->wgl.h_instance = GetModuleHandle(0);

    {
        WNDCLASS bootstrap_wnd_class = (WNDCLASS){
            .hInstance = win->wgl.h_instance,
            .lpfnWndProc = DefWindowProc,
            .lpszClassName = L"Bootstrap Window Class"
        };
        ATOM atom = RegisterClass(&bootstrap_wnd_class);

        HWND bootstrap_window = CreateWindow(
            bootstrap_wnd_class.lpszClassName,
            L"Bootstrap Win",
            0,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            0, 0, win->wgl.h_instance, 0
        );

        HDC bootstrap_dc = GetDC(bootstrap_window);

        PIXELFORMATDESCRIPTOR pixel_format_desc = (PIXELFORMATDESCRIPTOR){
            .nSize = sizeof(PIXELFORMATDESCRIPTOR),
            .nVersion = 1,
            .dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW,
            .iPixelType = PFD_TYPE_RGBA,
            .cColorBits = 32,
            .cDepthBits = 0,//24,
            .cStencilBits = 0,//8,
            .iLayerType = PFD_MAIN_PLANE
        };

        i32 pixel_format = ChoosePixelFormat(bootstrap_dc, &pixel_format_desc);
        SetPixelFormat(bootstrap_dc, pixel_format, &pixel_format_desc);

        HGLRC bootstrap_context = wglCreateContext(bootstrap_dc);
        wglMakeCurrent(bootstrap_dc, bootstrap_context);

        wglCreateContexAttribsARB = (wglCreateContextAttribsARB_func*)wgl_get_proc_address("wglCreateContextAttribsARB");
        wglChoosePixelFormatARB = (wglChoosePixelFormatARB_func*)wgl_get_proc_address("wglChoosePixelFormatARB");

        wglMakeCurrent(bootstrap_dc, NULL);
        wglDeleteContext(bootstrap_context);
        ReleaseDC(bootstrap_window, bootstrap_dc);
        UnregisterClass(bootstrap_wnd_class.lpszClassName, win->wgl.h_instance);
        DestroyWindow(bootstrap_window);
    }

    win->wgl.window_class = (WNDCLASS){
        .hInstance = win->wgl.h_instance,
        .lpfnWndProc = window_proc,
        .lpszClassName = L"OpenGL Window Class"
    };
    ATOM atom = RegisterClass(&win->wgl.window_class);

    string16 title16 = str16_from_str8(arena, title);

    RECT win_rect = { 0, 0, width, height };
    AdjustWindowRect(&win_rect, WS_OVERLAPPEDWINDOW, FALSE);
    
    win->wgl.window = CreateWindow(
        win->wgl.window_class.lpszClassName,
        title16.str,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        win_rect.right - win_rect.left, win_rect.bottom - win_rect.top,
        NULL, NULL, win->wgl.h_instance, NULL
    );

    SetPropA(win->wgl.window, "gfx_window", win);

    marena_pop(arena, sizeof(u16) * title16.size);

    win->wgl.device_context = GetDC(win->wgl.window);

    i32 pixel_format_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
        WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,         32,
        WGL_DEPTH_BITS_ARB,         0,//24,
        WGL_STENCIL_BITS_ARB,       0,//8,
        0
    };

    i32 pixel_format;
    u32 num_formats;
    wglChoosePixelFormatARB(win->wgl.device_context, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);

    PIXELFORMATDESCRIPTOR pfd;
    DescribePixelFormat(win->wgl.device_context, pixel_format, sizeof(pfd), &pfd);
    SetPixelFormat(win->wgl.device_context, pixel_format, &pfd);

    int gl_attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 4,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };

    win->wgl.context = wglCreateContexAttribsARB(win->wgl.device_context, NULL, gl_attribs);

    win->width = width;
    win->height = height;
    win->title = title;

    return win;
}
void gfx_win_make_current(gfx_window* win) {
    wglMakeCurrent(win->wgl.device_context, win->wgl.context);

    ShowWindow(win->wgl.window, SW_SHOW);
}

void gfx_win_destroy(gfx_window* win) {
    wglMakeCurrent(win->wgl.device_context, NULL);
    wglDeleteContext(win->wgl.context);
    ReleaseDC(win->wgl.window, win->wgl.device_context);
    UnregisterClass(win->wgl.window_class.lpszClassName, win->wgl.h_instance);
    DestroyWindow(win->wgl.window);
}

void gfx_win_swap_buffers(gfx_window* win) {
    SwapBuffers(win->wgl.device_context);
}
void gfx_win_process_events(gfx_window* win) {
    memcpy(win->prev_mouse_buttons, win->mouse_buttons, GFX_NUM_MOUSE_BUTTONS);
    memcpy(win->prev_keys, win->keys, GFX_NUM_KEYS);

    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            win->should_close = true;
        } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

void gfx_win_set_size(gfx_window* win, u32 width, u32 height) {
    win->width = width;
    win->height = height;

    RECT rect = (RECT){ 0, 0, width, height };
    i32 out = AdjustWindowRect(&rect, GetWindowLong(win->wgl.window, GWL_STYLE), false);

    SetWindowPos(win->wgl.window, NULL,
            0, 0, (i32)width, (i32)height,
            SWP_NOMOVE | SWP_DRAWFRAME);// | WS_VISIBLE);
}
void gfx_win_set_title(marena* arena, gfx_window* win, string8 title) {
    win->title = title;

    string16 title16 = str16_from_str8(arena, title);

    SetWindowText(win->wgl.window, title16.str);

    marena_pop(arena, sizeof(u16) * title16.size);
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    gfx_window* win = (gfx_window*)GetPropA(hwnd, "gfx_window");

    switch (uMsg) {
        case WM_MOUSEMOVE:
            win->mouse_pos.x = (f32)((lParam) & 0xffff);
            win->mouse_pos.y = (f32)((lParam >> 16) & 0xffff);
            break;

        case WM_LBUTTONDOWN:
            win->mouse_buttons[GFX_MB_LEFT] = true; break;
        case WM_LBUTTONUP:
            win->mouse_buttons[GFX_MB_LEFT] = false; break;
        case WM_MBUTTONDOWN:
            win->mouse_buttons[GFX_MB_MIDDLE] = true; break;
        case WM_MBUTTONUP:
            win->mouse_buttons[GFX_MB_MIDDLE] = false; break;
        case WM_RBUTTONDOWN:
            win->mouse_buttons[GFX_MB_RIGHT] = true; break;
        case WM_RBUTTONUP:
            win->mouse_buttons[GFX_MB_RIGHT] = false; break;

        case WM_KEYDOWN: break;
        case WM_KEYUP: break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void opengl_load_functions(gfx_window* win) {
    #define X(ret, name, args) name = (gl_func_##name)wgl_get_proc_address(#name);
        #include "opengl_xlist.h"
    #undef X
}

#endif // AP_OPENGL && _WIN32
