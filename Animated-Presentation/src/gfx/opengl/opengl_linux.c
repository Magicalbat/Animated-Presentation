#if defined(AP_OPENGL) && defined(AP_PLATFORM_LINUX)

#include "base/base.h"
#include "gfx/gfx.h"
#include "opengl.h"

#define X(ret, name, args) gl_func_##name##_t name = NULL;
    #include "opengl_xlist.h"
#undef X

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

static bool isExtensionSupported(const char *extList, const char *extension);

gfx_window_t* gfx_win_create(arena_t* arena, u32 width, u32 height, string8_t title) {
    gfx_window_t* win = CREATE_ZERO_STRUCT(win, gfx_window_t, arena);

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
        GLX_DEPTH_SIZE      , 24,
        GLX_STENCIL_SIZE    , 8,
        GLX_DOUBLEBUFFER    , True,
        None
    };   

    i32 fbcount;
    GLXFBConfig* fbc = glXChooseFBConfig(win->glx.display, DefaultScreen(win->glx.display), glx_attribs, &fbcount);
    if (fbc == 0) {
    	XCloseDisplay(win->glx.display);
        ASSERT(false, "Failed to retrieve framebuffer.");
    }

    int best_fbc_i = -1, worst_fbc_i = -1, best_num_samp = -1, worst_num_samp = 999;
    
    for (int i = 0; i < fbcount; ++i) {
        XVisualInfo *vi = glXGetVisualFromFBConfig(win->glx.display, fbc[i]);
        if (vi) {
            int samp_buf, samples;
            glXGetFBConfigAttrib(win->glx.display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
            glXGetFBConfigAttrib(win->glx.display, fbc[i], GLX_SAMPLES       , &samples );
        
            //printf( "  Matching fbconfig %d, visual ID 0x%2x: SAMPLE_BUFFERS = %d,"
            //        " SAMPLES = %d\n", 
            //        i, vi->visualid, samp_buf, samples );

            if (best_fbc_i < 0 || samp_buf && samples > best_num_samp)
                best_fbc_i = i, best_num_samp = samples;
            if (worst_fbc_i < 0 || !samp_buf || samples < worst_num_samp)
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
        .event_mask = ExposureMask | KeyPressMask
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
    XSelectInput(win->glx.display, win->glx.window, KeyPress | ClientMessage);
    
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

    u8* title_cstr = arena_alloc(arena, title.size + 1);
    memcpy(title_cstr, title.str, title.size);
    title_cstr[title.size] = '\0';
    XStoreName(win->glx.display, win->glx.window, title_cstr);
    arena_pop(arena, title.size + 1);
    
    win->info = (gfx_window_info_t){
        .mouse_pos = (vec2_t){ 0, 0 },
        .new_mouse_buttons = (b8*)arena_alloc(arena, sizeof(b8) * GFX_NUM_MOUSE_BUTTONS),
        .old_mouse_buttons = (b8*)arena_alloc(arena, sizeof(b8) * GFX_NUM_MOUSE_BUTTONS),
        .new_keys = (b8*)arena_alloc(arena, sizeof(b8) * GFX_NUM_KEYS),
        .old_keys = (b8*)arena_alloc(arena, sizeof(b8) * GFX_NUM_KEYS),
        .width = width,
        .height = height,
        .title = title,
        .should_close = false
    };

    return win;
}
void gfx_win_make_current(gfx_window_t* win) {
    glXMakeCurrent(win->glx.display, win->glx.window, win->glx.context);
}
void gfx_win_destroy(gfx_window_t* win) {
    glXDestroyContext(win->glx.display, win->glx.context);

    XDestroyWindow(win->glx.display, win->glx.window);
    XCloseDisplay(win->glx.display);
}

void gfx_win_swap_buffers(gfx_window_t* win) {
    glXSwapBuffers(win->glx.display, win->glx.window);
}
void gfx_win_process_events(gfx_window_t* win) {
    while (XPending(win->glx.display)) {
        XEvent e;
        XNextEvent(win->glx.display, &e);

        switch(e.type) {
            case KeyPress:
                break;
            case ClientMessage:
                if (e.xclient.data.l[0] == win->glx.del_window) {
                    win->info.should_close = true;
                }
                break;
            default:
                break;
        }
    }
}

void gfx_win_set_size(gfx_window_t* win, u32 width, u32 height) {
    win->info.width = width;
    win->info.height = height;
    XResizeWindow(win->glx.display, win->glx.window, width, height);
}
void gfx_win_set_title(arena_t* arena, gfx_window_t* win, string8_t title) {
    win->info.title = title;
    char* title_cstr = arena_alloc(arena, title.size + 1);
    memcpy(title_cstr, title.str, title.size);
    title_cstr[title.size] = '\0';
    
    XStoreName(win->glx.display, win->glx.window, title_cstr);
    
    arena_pop(arena, title.size + 1);
}

void opengl_load_functions(gfx_window_t* win) {
    #define X(ret, name, args) name = (gl_func_##name##_t)glXGetProcAddress(#name);
    #include "opengl_xlist.h"
    #undef X
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

#endif // AP_OPENGL && AP_PLATFORM_LINUX
