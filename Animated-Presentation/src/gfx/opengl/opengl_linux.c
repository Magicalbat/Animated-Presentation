#if defined(AP_OPENGL) && defined(__linux__)

#include "base/base.h"
#include "gfx/gfx.h"
#include "opengl.h"

#define X(ret, name, args) gl_func_##name name = NULL;
    #include "opengl_xlist.h"
#undef X

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

static bool isExtensionSupported(const char *extList, const char *extension);

gfx_window* gfx_win_create(marena* arena, u32 width, u32 height, string8 title) {
    gfx_window* win = CREATE_ZERO_STRUCT(arena, gfx_window);

    win->glx.display = XOpenDisplay(NULL);
    ASSERT(win->glx.display != NULL, "Cannot open display");

    win->glx.screen = DefaultScreen(win->glx.display);
    i32 major_glx, minor_glx = 0;
    glXQueryVersion(win->glx.display, &major_glx, &minor_glx);
    ASSERT(major_glx > 0 && minor_glx > 1, "GLX is not at a high enough version.");

    i32 glx_attribs[] = {
        GLX_X_RENDERABLE    , True,
        GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
        GLX_RENDER_TYPE     , GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
        GLX_RED_SIZE        , 8,
        GLX_GREEN_SIZE      , 8,
        GLX_BLUE_SIZE       , 8,
        GLX_ALPHA_SIZE      , 8,
        GLX_DEPTH_SIZE      , 0,//24,
        GLX_STENCIL_SIZE    , 0,//8,
        GLX_DOUBLEBUFFER    , True,
        None
    };   

    i32 fbcount;
    GLXFBConfig* fbc = glXChooseFBConfig(win->glx.display, DefaultScreen(win->glx.display), glx_attribs, &fbcount);
    if (fbc == 0) {
        XCloseDisplay(win->glx.display);
        log_error("Failed to retrieve framebuffer");
    }

    int best_fbc_i = -1, worst_fbc_i = -1, best_num_samp = -1, worst_num_samp = 999;
    
    for (int i = 0; i < fbcount; ++i) {
        XVisualInfo *vi = glXGetVisualFromFBConfig(win->glx.display, fbc[i]);
        if (vi) {
            int samp_buf, samples;
            glXGetFBConfigAttrib(win->glx.display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
            glXGetFBConfigAttrib(win->glx.display, fbc[i], GLX_SAMPLES       , &samples );
        
            if (best_fbc_i < 0 || (samp_buf && samples > best_num_samp))
                best_fbc_i = i, best_num_samp = samples;
            if (worst_fbc_i < 0 || (!samp_buf || samples < worst_num_samp))
                worst_fbc_i = i, worst_num_samp = samples;
      }
      XFree(vi);
    }
    
    win->glx.fb_config = fbc[best_fbc_i];

    XFree(fbc);

    XVisualInfo* visual = glXGetVisualFromFBConfig(win->glx.display, win->glx.fb_config);
    if (visual == NULL) {
        ASSERT(false, "Could not create correct visual window.");
        XCloseDisplay(win->glx.display);
    }

    XSetWindowAttributes window_attribs = (XSetWindowAttributes){
        .border_pixel = BlackPixel(win->glx.display, win->glx.screen),
        .background_pixel = WhitePixel(win->glx.display, win->glx.screen),
        .override_redirect = True,
        .colormap = XCreateColormap(win->glx.display, RootWindow(win->glx.display, win->glx.screen), visual->visual, AllocNone),
        .event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | KeyPressMask | KeyReleaseMask
    };

    // https://stackoverflow.com/questions/10792361/how-do-i-gracefully-exit-an-x11-event-loop
    // If you want a different behavior (that is, to capture the closing event from the Window Manager), you need to use the WM_DESTROY_WINDOW protocol.
    win->glx.window = XCreateWindow(
        win->glx.display, RootWindow(win->glx.display, win->glx.screen),
        0, 0, width, height, 0,
        visual->depth, InputOutput, visual->visual,
        CWBackPixel | CWColormap | CWBorderPixel | CWEventMask,
        &window_attribs
    );
    
    win->glx.del_window = XInternAtom(win->glx.display, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(win->glx.display , win->glx.window, &win->glx.del_window, 1);
    XMapWindow(win->glx.display, win->glx.window);
    
    XFree(visual);

    const char *glx_exts = glXQueryExtensionsString(win->glx.display, DefaultScreen(win->glx.display));
    
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc) glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );

    i32 context_attribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 4,
        GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        None
    };

    //win->glx.context = 0;
    if (!isExtensionSupported( glx_exts, "GLX_ARB_create_context")) {
        win->glx.context = glXCreateNewContext(win->glx.display, win->glx.fb_config, GLX_RGBA_TYPE, 0, true);
    } else {
        win->glx.context = glXCreateContextAttribsARB(win->glx.display, win->glx.fb_config, 0, true, context_attribs);
    }

    XSync(win->glx.display, false);

    XFreeColormap(win->glx.display, window_attribs.colormap);

    marena_temp temp = marena_temp_begin(arena);
    u8* title_cstr = str8_to_cstr(temp.arena, title);
    XStoreName(win->glx.display, win->glx.window, (char*)title_cstr);
    marena_temp_end(temp);
    
    win->width = width;
    win->height = height;
    win->title = title;

    return win;
}
void gfx_win_make_current(gfx_window* win) {
    glXMakeCurrent(win->glx.display, win->glx.window, win->glx.context);
    
    glViewport(0, 0, win->width, win->height);
}
void gfx_win_destroy(gfx_window* win) {
    glXDestroyContext(win->glx.display, win->glx.context);

    XDestroyWindow(win->glx.display, win->glx.window);
    XCloseDisplay(win->glx.display);
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
    glXSwapBuffers(win->glx.display, win->glx.window);
}

static gfx_key x11_translate_key(XKeyEvent* e);
void gfx_win_process_events(gfx_window* win) {
    memcpy(win->prev_mouse_buttons, win->mouse_buttons, GFX_NUM_MOUSE_BUTTONS);
    memcpy(win->prev_keys, win->keys, GFX_NUM_KEYS);
    
    while (XPending(win->glx.display)) {
        XEvent e;
        XNextEvent(win->glx.display, &e);

        switch(e.type) {
            case ButtonPress:
                win->mouse_buttons[e.xbutton.button - 1] = true;
                break;
            case ButtonRelease:
                win->mouse_buttons[e.xbutton.button - 1] = false;
                break;
            case MotionNotify:
                win->mouse_pos.x = (f32)e.xmotion.x;
                win->mouse_pos.y = (f32)e.xmotion.y;
                break;
            case KeyPress: ;
                gfx_key keydown = x11_translate_key(&e.xkey);
                win->keys[keydown] = true;
                break;
            case KeyRelease: ;
                gfx_key keyup = x11_translate_key(&e.xkey);
                win->keys[keyup] = false;
                break;
            case ClientMessage:
                if (e.xclient.data.l[0] == win->glx.del_window) {
                    win->should_close = true;
                }
                break;
            default:
                break;
        }
    }
}

void gfx_win_set_size(gfx_window* win, u32 width, u32 height) {
    win->width = width;
    win->height = height;
    XResizeWindow(win->glx.display, win->glx.window, width, height);
    glViewport(0, 0, width, height);
}
void gfx_win_set_title(gfx_window* win, string8 title) {
    win->title = title;

    marena_temp scratch = marena_scratch_get(NULL, 0);
    u8* title_cstr = str8_to_cstr(scratch.arena, title);
    
    XStoreName(win->glx.display, win->glx.window, (char*)title_cstr);

    marena_scratch_release(scratch);
}

void opengl_load_functions(gfx_window* win) {
    #define X(ret, name, args) name = (gl_func_##name)glXGetProcAddress(#name);
    #include "opengl_xlist.h"
    #undef X
}

// Adapted from sokol_app.h
// https://github.com/floooh/sokol/blob/master/sokol_app.h#L10175 
static gfx_key x11_translate_key(XKeyEvent* e) {
    KeySym keysym = XLookupKeysym(e, 0);

    switch (keysym) {
        case XK_Escape:         return GFX_KEY_ESCAPE;
        case XK_Tab:            return GFX_KEY_TAB;
        case XK_Shift_L:        return GFX_KEY_LSHIFT;
        case XK_Shift_R:        return GFX_KEY_RSHIFT;
        case XK_Control_L:      return GFX_KEY_LCONTROL;
        case XK_Control_R:      return GFX_KEY_RCONTROL;
        case XK_Meta_L:
        case XK_Alt_L:          return GFX_KEY_LALT;
        case XK_Mode_switch:    /* Mapped to Alt_R on many keyboards */
        case XK_ISO_Level3_Shift: /* AltGr on at least some machines */
        case XK_Meta_R:
        case XK_Alt_R:          return GFX_KEY_RALT;
        case XK_Num_Lock:       return GFX_KEY_NUM_LOCK;
        case XK_Caps_Lock:      return GFX_KEY_CAPSLOCK;
        case XK_Scroll_Lock:    return GFX_KEY_SCROLL_LOCK;
        case XK_Delete:         return GFX_KEY_DELETE;
        case XK_BackSpace:      return GFX_KEY_BACKSPACE;
        case XK_Return:         return GFX_KEY_ENTER;
        case XK_Home:           return GFX_KEY_HOME;
        case XK_End:            return GFX_KEY_END;
        case XK_Page_Up:        return GFX_KEY_PAGEUP;
        case XK_Page_Down:      return GFX_KEY_PAGEDOWN;
        case XK_Insert:         return GFX_KEY_INSERT;
        case XK_Left:           return GFX_KEY_LEFT;
        case XK_Right:          return GFX_KEY_RIGHT;
        case XK_Down:           return GFX_KEY_DOWN;
        case XK_Up:             return GFX_KEY_UP;
        case XK_F1:             return GFX_KEY_F1;
        case XK_F2:             return GFX_KEY_F2;
        case XK_F3:             return GFX_KEY_F3;
        case XK_F4:             return GFX_KEY_F4;
        case XK_F5:             return GFX_KEY_F5;
        case XK_F6:             return GFX_KEY_F6;
        case XK_F7:             return GFX_KEY_F7;
        case XK_F8:             return GFX_KEY_F8;
        case XK_F9:             return GFX_KEY_F9;
        case XK_F10:            return GFX_KEY_F10;
        case XK_F11:            return GFX_KEY_F11;
        case XK_F12:            return GFX_KEY_F12;

        case XK_KP_Divide:      return GFX_KEY_NUMPAD_DIVIDE;
        case XK_KP_Multiply:    return GFX_KEY_NUMPAD_MULTIPLY;
        case XK_KP_Subtract:    return GFX_KEY_NUMPAD_SUBTRACT;
        case XK_KP_Add:         return GFX_KEY_NUMPAD_ADD;

        case XK_KP_Insert:      return GFX_KEY_NUMPAD_0;
        case XK_KP_End:         return GFX_KEY_NUMPAD_1;
        case XK_KP_Down:        return GFX_KEY_NUMPAD_2;
        case XK_KP_Page_Down:   return GFX_KEY_NUMPAD_3;
        case XK_KP_Left:        return GFX_KEY_NUMPAD_4;
        case XK_KP_Begin:       return GFX_KEY_NUMPAD_5;
        case XK_KP_Right:       return GFX_KEY_NUMPAD_6;
        case XK_KP_Home:        return GFX_KEY_NUMPAD_7;
        case XK_KP_Up:          return GFX_KEY_NUMPAD_8;
        case XK_KP_Page_Up:     return GFX_KEY_NUMPAD_9;
        case XK_KP_Delete:      return GFX_KEY_NUMPAD_DECIMAL;
        case XK_KP_Enter:       return GFX_KEY_NUMPAD_ENTER;

        case XK_a:              return GFX_KEY_A;
        case XK_b:              return GFX_KEY_B;
        case XK_c:              return GFX_KEY_C;
        case XK_d:              return GFX_KEY_D;
        case XK_e:              return GFX_KEY_E;
        case XK_f:              return GFX_KEY_F;
        case XK_g:              return GFX_KEY_G;
        case XK_h:              return GFX_KEY_H;
        case XK_i:              return GFX_KEY_I;
        case XK_j:              return GFX_KEY_J;
        case XK_k:              return GFX_KEY_K;
        case XK_l:              return GFX_KEY_L;
        case XK_m:              return GFX_KEY_M;
        case XK_n:              return GFX_KEY_N;
        case XK_o:              return GFX_KEY_O;
        case XK_p:              return GFX_KEY_P;
        case XK_q:              return GFX_KEY_Q;
        case XK_r:              return GFX_KEY_R;
        case XK_s:              return GFX_KEY_S;
        case XK_t:              return GFX_KEY_T;
        case XK_u:              return GFX_KEY_U;
        case XK_v:              return GFX_KEY_V;
        case XK_w:              return GFX_KEY_W;
        case XK_x:              return GFX_KEY_X;
        case XK_y:              return GFX_KEY_Y;
        case XK_z:              return GFX_KEY_Z;
        case XK_1:              return GFX_KEY_1;
        case XK_2:              return GFX_KEY_2;
        case XK_3:              return GFX_KEY_3;
        case XK_4:              return GFX_KEY_4;
        case XK_5:              return GFX_KEY_5;
        case XK_6:              return GFX_KEY_6;
        case XK_7:              return GFX_KEY_7;
        case XK_8:              return GFX_KEY_8;
        case XK_9:              return GFX_KEY_9;
        case XK_0:              return GFX_KEY_0;
        case XK_space:          return GFX_KEY_SPACE;
        case XK_minus:          return GFX_KEY_MINUS;
        case XK_equal:          return GFX_KEY_EQUAL;
        case XK_bracketleft:    return GFX_KEY_LBRACKET;
        case XK_bracketright:   return GFX_KEY_RBRACKET;
        case XK_backslash:      return GFX_KEY_BACKSLASH;
        case XK_semicolon:      return GFX_KEY_SEMICOLON;
        case XK_apostrophe:     return GFX_KEY_APOSTROPHE;
        case XK_grave:          return GFX_KEY_BACKTICK;
        case XK_comma:          return GFX_KEY_COMMA;
        case XK_period:         return GFX_KEY_PERIOD;
        case XK_slash:          return GFX_KEY_FORWARDSLASH;
        default:                return GFX_KEY_NONE;
    }

    return GFX_KEY_NONE;
}

// Helper to check for extension string presence.  Adapted from:
//   http://www.opengl.org/resources/features/OGLextensions/
static bool isExtensionSupported(const char *extList, const char *extension) {
    const char *start;
    const char *where, *terminator;

    /* Extension names should not have spaces. */
    where = strchr(extension, ' ');
    if (where || *extension == '\0')
    return false;

    /* It takes a bit of care to be fool-proof about parsing the
     OpenGL extensions string. Don't be fooled by sub-strings,
     etc. */
    for (start=extList;;) {
        where = strstr(start, extension);

        if (!where) {
             break;
        }

        terminator = where + strlen(extension);

        if ( where == start || *(where - 1) == ' ' ) {
            if ( *terminator == ' ' || *terminator == '\0' ) {
                return true;
            }
        }    

        start = terminator;
    }

    return false;
}

#endif // AP_OPENGL && __linux__