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
static void win32_init_keymap(gfx_window* win);

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
        .lpszClassName = L"OpenGL Window Class",
        .hCursor = LoadCursor(NULL, IDC_ARROW)
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

    win32_init_keymap(win);

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
            gfx_key down_key = win->wgl.keymap[wParam];
            win->keys[down_key] = true;
            break;
        case WM_KEYUP:
            gfx_key up_key = win->wgl.keymap[wParam];
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

static void win32_init_keymap(gfx_window* win) {
    memset(win->wgl.keymap, 0, sizeof(win->wgl.keymap));

    win->wgl.keymap[VK_BACK] = GFX_KEY_BACKSPACE;
    win->wgl.keymap[VK_TAB] = GFX_KEY_TAB;
    win->wgl.keymap[VK_RETURN] = GFX_KEY_ENTER;
    win->wgl.keymap[VK_CAPITAL] = GFX_KEY_CAPSLOCK;
    win->wgl.keymap[VK_ESCAPE] = GFX_KEY_ESCAPE;
    win->wgl.keymap[VK_SPACE] = GFX_KEY_SPACE;
    win->wgl.keymap[VK_PRIOR] = GFX_KEY_PAGEUP;
    win->wgl.keymap[VK_NEXT] = GFX_KEY_PAGEDOWN;
    win->wgl.keymap[VK_END] = GFX_KEY_END;
    win->wgl.keymap[VK_HOME] = GFX_KEY_HOME;
    win->wgl.keymap[VK_LEFT] = GFX_KEY_LEFT;
    win->wgl.keymap[VK_UP] = GFX_KEY_UP;
    win->wgl.keymap[VK_RIGHT] = GFX_KEY_RIGHT;
    win->wgl.keymap[VK_DOWN] = GFX_KEY_DOWN;
    win->wgl.keymap[VK_INSERT] = GFX_KEY_INSERT;
    win->wgl.keymap[VK_DELETE] = GFX_KEY_DELETE;
    win->wgl.keymap[0x30] = GFX_KEY_0;
    win->wgl.keymap[0x31] = GFX_KEY_1;
    win->wgl.keymap[0x32] = GFX_KEY_2;
    win->wgl.keymap[0x33] = GFX_KEY_3;
    win->wgl.keymap[0x34] = GFX_KEY_4;
    win->wgl.keymap[0x35] = GFX_KEY_5;
    win->wgl.keymap[0x36] = GFX_KEY_6;
    win->wgl.keymap[0x37] = GFX_KEY_7;
    win->wgl.keymap[0x38] = GFX_KEY_8;
    win->wgl.keymap[0x39] = GFX_KEY_9;
    win->wgl.keymap[0x41] = GFX_KEY_A;
    win->wgl.keymap[0x42] = GFX_KEY_B;
    win->wgl.keymap[0x43] = GFX_KEY_C;
    win->wgl.keymap[0x44] = GFX_KEY_D;
    win->wgl.keymap[0x45] = GFX_KEY_E;
    win->wgl.keymap[0x46] = GFX_KEY_F;
    win->wgl.keymap[0x47] = GFX_KEY_G;
    win->wgl.keymap[0x48] = GFX_KEY_H;
    win->wgl.keymap[0x49] = GFX_KEY_I;
    win->wgl.keymap[0x4A] = GFX_KEY_J;
    win->wgl.keymap[0x4B] = GFX_KEY_K;
    win->wgl.keymap[0x4C] = GFX_KEY_L;
    win->wgl.keymap[0x4D] = GFX_KEY_M;
    win->wgl.keymap[0x4E] = GFX_KEY_N;
    win->wgl.keymap[0x4F] = GFX_KEY_O;
    win->wgl.keymap[0x50] = GFX_KEY_P;
    win->wgl.keymap[0x51] = GFX_KEY_Q;
    win->wgl.keymap[0x52] = GFX_KEY_R;
    win->wgl.keymap[0x53] = GFX_KEY_S;
    win->wgl.keymap[0x54] = GFX_KEY_T;
    win->wgl.keymap[0x55] = GFX_KEY_U;
    win->wgl.keymap[0x56] = GFX_KEY_V;
    win->wgl.keymap[0x57] = GFX_KEY_W;
    win->wgl.keymap[0x58] = GFX_KEY_X;
    win->wgl.keymap[0x59] = GFX_KEY_Y;
    win->wgl.keymap[0x5A] = GFX_KEY_Z;
    win->wgl.keymap[VK_NUMPAD0] = GFX_KEY_NUMPAD_0;
    win->wgl.keymap[VK_NUMPAD1] = GFX_KEY_NUMPAD_1;
    win->wgl.keymap[VK_NUMPAD2] = GFX_KEY_NUMPAD_2;
    win->wgl.keymap[VK_NUMPAD3] = GFX_KEY_NUMPAD_3;
    win->wgl.keymap[VK_NUMPAD4] = GFX_KEY_NUMPAD_4;
    win->wgl.keymap[VK_NUMPAD5] = GFX_KEY_NUMPAD_5;
    win->wgl.keymap[VK_NUMPAD6] = GFX_KEY_NUMPAD_6;
    win->wgl.keymap[VK_NUMPAD7] = GFX_KEY_NUMPAD_7;
    win->wgl.keymap[VK_NUMPAD8] = GFX_KEY_NUMPAD_8;
    win->wgl.keymap[VK_NUMPAD9] = GFX_KEY_NUMPAD_9;
    win->wgl.keymap[VK_MULTIPLY] = GFX_KEY_NUMPAD_MULTIPLY;
    win->wgl.keymap[VK_ADD] = GFX_KEY_NUMPAD_ADD;
    win->wgl.keymap[VK_SUBTRACT] = GFX_KEY_NUMPAD_SUBTRACT;
    win->wgl.keymap[VK_DECIMAL] = GFX_KEY_NUMPAD_DECIMAL;
    win->wgl.keymap[VK_DIVIDE] = GFX_KEY_NUMPAD_DIVIDE;
    win->wgl.keymap[VK_F1] = GFX_KEY_F1;
    win->wgl.keymap[VK_F2] = GFX_KEY_F2;
    win->wgl.keymap[VK_F3] = GFX_KEY_F3;
    win->wgl.keymap[VK_F4] = GFX_KEY_F4;
    win->wgl.keymap[VK_F5] = GFX_KEY_F5;
    win->wgl.keymap[VK_F6] = GFX_KEY_F6;
    win->wgl.keymap[VK_F7] = GFX_KEY_F7;
    win->wgl.keymap[VK_F8] = GFX_KEY_F8;
    win->wgl.keymap[VK_F9] = GFX_KEY_F9;
    win->wgl.keymap[VK_F10] = GFX_KEY_F10;
    win->wgl.keymap[VK_F11] = GFX_KEY_F11;
    win->wgl.keymap[VK_F12] = GFX_KEY_F12;
    win->wgl.keymap[VK_NUMLOCK] = GFX_KEY_NUM_LOCK;
    win->wgl.keymap[VK_SCROLL] = GFX_KEY_SCROLL_LOCK;
    win->wgl.keymap[VK_LSHIFT] = GFX_KEY_LSHIFT;
    win->wgl.keymap[VK_RSHIFT] = GFX_KEY_RSHIFT;
    win->wgl.keymap[VK_LCONTROL] = GFX_KEY_LCONTROL;
    win->wgl.keymap[VK_RCONTROL] = GFX_KEY_RCONTROL;
    win->wgl.keymap[VK_LMENU] = GFX_KEY_LALT;
    win->wgl.keymap[VK_RMENU] = GFX_KEY_RALT;
    win->wgl.keymap[VK_OEM_1] = GFX_KEY_SEMICOLON;
    win->wgl.keymap[VK_OEM_PLUS] = GFX_KEY_EQUAL;
    win->wgl.keymap[VK_OEM_COMMA] = GFX_KEY_COMMA;
    win->wgl.keymap[VK_OEM_MINUS] = GFX_KEY_MINUS;
    win->wgl.keymap[VK_OEM_PERIOD] = GFX_KEY_PERIOD;
    win->wgl.keymap[VK_OEM_2] = GFX_KEY_FORWARDSLASH;
    win->wgl.keymap[VK_OEM_3] = GFX_KEY_BACKTICK;
    win->wgl.keymap[VK_OEM_4] = GFX_KEY_LBRACKET;
    win->wgl.keymap[VK_OEM_5] = GFX_KEY_BACKSLASH;
    win->wgl.keymap[VK_OEM_6] = GFX_KEY_RBRACKET;
    win->wgl.keymap[VK_OEM_7] = GFX_KEY_APOSTROPHE;
}

#endif // AP_OPENGL && _WIN32
