#if defined(AP_OPENGL) && defined(AP_PLATFORM_WINDOWS)

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

// https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions#Windows_2
static void* wgl_get_proc_address(const char* name) {
	void* p = (void*)wglGetProcAddress(name);
	if (p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1)) {
		HMODULE module = LoadLibrary(L"opengl32.dll");
		p = (void*)GetProcAddress(module, name);
	}
	return p;
}

static LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef HGLRC WINAPI wglCreateContextAttribsARB_t(HDC hdc, HGLRC hShareContext, const int *attribList);
typedef BOOL WINAPI wglChoosePixelFormatARB_t(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);

static wglCreateContextAttribsARB_t *wglCreateContexAttribsARB = NULL;
static wglChoosePixelFormatARB_t    *wglChoosePixelFormatARB   = NULL;

gfx_window_t* gfx_win_create(arena_t* arena, u32 width, u32 height, string8_t title) {
    gfx_window_t* win = CREATE_ZERO_STRUCT(win, gfx_window_t, arena);

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
            NULL,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            NULL, NULL, win->wgl.h_instance, NULL
        );

        HDC bootstrap_dc = GetDC(bootstrap_window);

        // TODO: standardize or add morme options to pixel format on both
        PIXELFORMATDESCRIPTOR pixel_format_desc = (PIXELFORMATDESCRIPTOR){
            .nSize = sizeof(PIXELFORMATDESCRIPTOR),
            .nVersion = 1,
            .dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW,
            .iPixelType = PFD_TYPE_RGBA,
            .cColorBits = 32,
            .cDepthBits = 24,
            .cStencilBits = 8,
            .iLayerType = PFD_MAIN_PLANE
        };

        i32 pixel_format = ChoosePixelFormat(bootstrap_dc, &pixel_format_desc);
        SetPixelFormat(bootstrap_dc, pixel_format, &pixel_format_desc);

        HGLRC bootstrap_context = wglCreateContext(bootstrap_dc);
        wglMakeCurrent(bootstrap_dc, bootstrap_context);

        wglCreateContexAttribsARB = (wglCreateContextAttribsARB_t*)wgl_get_proc_address("wglCreateContextAttribsARB");
        wglChoosePixelFormatARB = (wglChoosePixelFormatARB_t*)wgl_get_proc_address("wglChoosePixelFormatARB");

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

    // TODO: replace this after string conversions are made
    u16* w_title = arena_alloc(arena, (title.size + 1) * sizeof(u16));
    mbstowcs(w_title, title.str, title.size + 1);

    win->wgl.window = CreateWindow(
        win->wgl.window_class.lpszClassName,
        w_title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        width, height,
        NULL, NULL, win->wgl.h_instance, NULL
    );

    win->wgl.device_context = GetDC(win->wgl.window);

    i32 pixel_format_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
        WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,         32,
        WGL_DEPTH_BITS_ARB,         24,
        WGL_STENCIL_BITS_ARB,       8,
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

    return win;
}
void gfx_win_make_current(gfx_window_t* win) {
    wglMakeCurrent(win->wgl.device_context, win->wgl.context);

    ShowWindow(win->wgl.window, SW_SHOW);
}

void gfx_win_destroy(gfx_window_t* win) {
    wglMakeCurrent(win->wgl.device_context, NULL);
    wglDeleteContext(win->wgl.context);
    ReleaseDC(win->wgl.window, win->wgl.device_context);
    UnregisterClass(win->wgl.window_class.lpszClassName, win->wgl.h_instance);
    DestroyWindow(win->wgl.window);
}

void gfx_win_swap_buffers(gfx_window_t* win) {
    SwapBuffers(win->wgl.device_context);
}

void gfx_win_set_size(gfx_window_t* win, u32 width, u32 height) {
}
void gfx_win_set_title(arena_t* arena, gfx_window_t* win, string8_t title) {
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

#endif // AP_OPENGL && AP_PLATFORM_WINDOWS
