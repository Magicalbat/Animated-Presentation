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
    
    glViewport(0, 0, win->width, win->height);
}

void gfx_win_destroy(gfx_window* win) {
    wglMakeCurrent(win->wgl.device_context, NULL);
    wglDeleteContext(win->wgl.context);
    ReleaseDC(win->wgl.window, win->wgl.device_context);
    UnregisterClass(win->wgl.window_class.lpszClassName, win->wgl.h_instance);
    DestroyWindow(win->wgl.window);
}

void gfx_win_clear_color(gfx_window* win, vec3 col) {
    win->clear_col = col;
    glClearColor(col.x, col.y, col.z, 1.0f);
}
void gfx_win_clear(gfx_window* win) {
    glClear(GL_COLOR_BUFFER_BIT);
}
void gfx_win_alpha_blend(gfx_window* win, b32 enable) {
    if (enable) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    } else {
        glDisable(GL_BLEND);
    }
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

    glViewport(0, 0, width, height);
}
void gfx_win_set_title(gfx_window* win, string8 title) {
    win->title = title;

    marena_temp scratch = marena_scratch_get(NULL, 0);
    
    string16 title16 = str16_from_str8(scratch.arena, title);
    
    SetWindowText(win->wgl.window, title16.str);

    marena_scratch_release(scratch);
}

gfx_key win32_translate_key(u32 key);

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

        case WM_KEYDOWN:
            gfx_key down_key = win32_translate_key((u32)wParam);
            win->keys[down_key] = true;
            break;
        case WM_KEYUP:
            gfx_key up_key = win32_translate_key((u32)wParam);
            win->keys[up_key] = false;
            break;

        case WM_SIZE:
            u32 width = (u32)LOWORD(lParam);
            u32 height = (u32)HIWORD(lParam);

            win->width = width;
            win->height = height;
            glViewport(0, 0, width, height);

            break;

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

gfx_key win32_translate_key(u32 key) {
    switch(key) {
        case VK_TAB: return GFX_KEY_TAB;
        case VK_RETURN: return GFX_KEY_ENTER;
        case VK_CAPITAL: return GFX_KEY_CAPSLOCK;
        case VK_ESCAPE: return GFX_KEY_ESCAPE;
        case VK_SPACE: return GFX_KEY_SPACE;
        case VK_PRIOR: return GFX_KEY_PAGEUP;
        case VK_NEXT: return GFX_KEY_PAGEDOWN;
        case VK_END: return GFX_KEY_END;
        case VK_HOME: return GFX_KEY_HOME;
        case VK_LEFT: return GFX_KEY_LEFT;
        case VK_UP: return GFX_KEY_UP;
        case VK_RIGHT: return GFX_KEY_RIGHT;
        case VK_DOWN: return GFX_KEY_DOWN;
        case VK_INSERT: return GFX_KEY_INSERT;
        case VK_DELETE: return GFX_KEY_DELETE;
        case 0x30: return GFX_KEY_0;
        case 0x31: return GFX_KEY_1;
        case 0x32: return GFX_KEY_2;
        case 0x33: return GFX_KEY_3;
        case 0x34: return GFX_KEY_4;
        case 0x35: return GFX_KEY_5;
        case 0x36: return GFX_KEY_6;
        case 0x37: return GFX_KEY_7;
        case 0x38: return GFX_KEY_8;
        case 0x39: return GFX_KEY_9;
        case 0x41: return GFX_KEY_A;
        case 0x42: return GFX_KEY_B;
        case 0x43: return GFX_KEY_C;
        case 0x44: return GFX_KEY_D;
        case 0x45: return GFX_KEY_E;
        case 0x46: return GFX_KEY_F;
        case 0x47: return GFX_KEY_G;
        case 0x48: return GFX_KEY_H;
        case 0x49: return GFX_KEY_I;
        case 0x4A: return GFX_KEY_J;
        case 0x4B: return GFX_KEY_K;
        case 0x4C: return GFX_KEY_L;
        case 0x4D: return GFX_KEY_M;
        case 0x4E: return GFX_KEY_N;
        case 0x4F: return GFX_KEY_O;
        case 0x50: return GFX_KEY_P;
        case 0x51: return GFX_KEY_Q;
        case 0x52: return GFX_KEY_R;
        case 0x53: return GFX_KEY_S;
        case 0x54: return GFX_KEY_T;
        case 0x55: return GFX_KEY_U;
        case 0x56: return GFX_KEY_V;
        case 0x57: return GFX_KEY_W;
        case 0x58: return GFX_KEY_X;
        case 0x59: return GFX_KEY_Y;
        case 0x5A: return GFX_KEY_Z;
        case VK_NUMPAD0: return GFX_KEY_NUMPAD0;
        case VK_NUMPAD1: return GFX_KEY_NUMPAD1;
        case VK_NUMPAD2: return GFX_KEY_NUMPAD2;
        case VK_NUMPAD3: return GFX_KEY_NUMPAD3;
        case VK_NUMPAD4: return GFX_KEY_NUMPAD4;
        case VK_NUMPAD5: return GFX_KEY_NUMPAD5;
        case VK_NUMPAD6: return GFX_KEY_NUMPAD6;
        case VK_NUMPAD7: return GFX_KEY_NUMPAD7;
        case VK_NUMPAD8: return GFX_KEY_NUMPAD8;
        case VK_NUMPAD9: return GFX_KEY_NUMPAD9;
        case VK_MULTIPLY: return GFX_KEY_MULTIPLY;
        case VK_ADD: return GFX_KEY_ADD;
        case VK_SUBTRACT: return GFX_KEY_SUBTRACT;
        case VK_DECIMAL: return GFX_KEY_DECIMAL;
        case VK_DIVIDE: return GFX_KEY_DIVIDE;
        case VK_F1: return GFX_KEY_F1;
        case VK_F2: return GFX_KEY_F2;
        case VK_F3: return GFX_KEY_F3;
        case VK_F4: return GFX_KEY_F4;
        case VK_F5: return GFX_KEY_F5;
        case VK_F6: return GFX_KEY_F6;
        case VK_F7: return GFX_KEY_F7;
        case VK_F8: return GFX_KEY_F8;
        case VK_F9: return GFX_KEY_F9;
        case VK_F10: return GFX_KEY_F10;
        case VK_F11: return GFX_KEY_F11;
        case VK_F12: return GFX_KEY_F12;
        case VK_NUMLOCK: return GFX_KEY_NUMLOCK;
        case VK_SCROLL: return GFX_KEY_SCROLLLOCK;
        case VK_LSHIFT: return GFX_KEY_LSHIFT;
        case VK_RSHIFT: return GFX_KEY_RSHIFT;
        case VK_LCONTROL: return GFX_KEY_LCONTROL;
        case VK_RCONTROL: return GFX_KEY_RCONTROL;
        case VK_LMENU: return GFX_KEY_LALT;
        case VK_RMENU: return GFX_KEY_RALT;
        case VK_OEM_1: return GFX_KEY_SEMICOLON;
        case VK_OEM_PLUS: return GFX_KEY_PLUS;
        case VK_OEM_COMMA: return GFX_KEY_COMMA;
        case VK_OEM_MINUS: return GFX_KEY_MINUS;
        case VK_OEM_PERIOD: return GFX_KEY_PERIOD;
        case VK_OEM_2: return GFX_KEY_FORWARDSLASH;
        case VK_OEM_3: return GFX_KEY_BACKTICK;
        case VK_OEM_4: return GFX_KEY_LBRACKET;
        case VK_OEM_5: return GFX_KEY_BACKSLASH;
        case VK_OEM_6: return GFX_KEY_RBRACKET;
        case VK_OEM_7: return GFX_KEY_APOSTROPHE;
    }

    return GFX_KEY_NONE;
}

#endif // AP_OPENGL && _WIN32
