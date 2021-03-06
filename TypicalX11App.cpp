// TypicalX11App.cpp --- An X11 application                     -*- C++ -*-
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// X Window System-related

#include <X11/Xlib.h>   // X library
#include <X11/Xutil.h>  // X11 utility
#include <X11/xpm.h>    // Xpm

//////////////////////////////////////////////////////////////////////////////
// C++ headers

#include <new>          // for std::bad_alloc
#include <string>       // for std::string
#include <vector>       // for std::vector

//////////////////////////////////////////////////////////////////////////////
// C headers

#include <cstdio>       // for std::printf, std::fprintf, ...
#include <cstring>      // for std::strlen, ...

//////////////////////////////////////////////////////////////////////////////
// private headers

#include "config.h"
#include "TypicalX11App.h"
#include "TypicalX11App.xpm"
#include "TypicalX11AppMask.xbm"

//////////////////////////////////////////////////////////////////////////////

// size of window
#define WIN_WIDTH   640
#define WIN_HEIGHT  400

// border width
#define BORDER_WIDTH 1

//////////////////////////////////////////////////////////////////////////////

// the application
struct X11App {
    int             m_argc;             // number of command line paramters
    char **         m_argv;             // command line paramters

    Display *       m_disp;             // X11 display
    Window          m_win;              // the main window
    bool            m_quit;             // quit flag
    bool            m_shift_pressed;    // Is [Shift] key pressed?
    bool            m_ctrl_pressed;     // Is [Ctrl] key pressed?

    std::string     m_option;           // option for example
    std::vector<std::string> m_files;   // target files

    Atom            m_wm_delete_window; // WM_DELETE_WINDOW atom

    Window          m_root_win;         // the root window

    Pixmap          m_icon_pixmap;      // icon pixmap
    Pixmap          m_icon_mask;        // icon mask

    GC              m_gc1;              // graphic context
    GC              m_gc2;              // graphic context
    GC              m_gc3;              // graphic context

    X11App(int argc, char **argv) :
        m_argc(argc),
        m_argv(argv),
        m_disp(NULL),
        m_win(None),
        m_quit(false),
        m_shift_pressed(false),
        m_ctrl_pressed(false),
        m_option("(none)"),
        m_root_win(None),
        m_icon_pixmap(None),
        m_icon_mask(None)
    {
    }

    ~X11App() {
        if (m_disp != NULL) {
            if (m_win != None) {
                // free icon pixmap
                if (m_icon_pixmap != None) {
                    XFreePixmap(m_disp, m_icon_pixmap);
                }
                if (m_icon_mask != None) {
                    XFreePixmap(m_disp, m_icon_mask);
                }

                // free graphic contexts
                XFreeGC(m_disp, m_gc1);
                XFreeGC(m_disp, m_gc2);
                XFreeGC(m_disp, m_gc3);
            }

            // close display
            XCloseDisplay(m_disp);
        }
    } // ~X11App

    bool load_icon_pixmap() {
        m_icon_pixmap = None;
        int status = XpmCreatePixmapFromData(
            m_disp, m_root_win, (char **)TypicalX11App_xpm,
            &m_icon_pixmap,
            NULL, NULL);
        m_icon_mask = XCreatePixmapFromBitmapData(
            m_disp, m_root_win, (char *)TypicalX11AppMask_bits,
            TypicalX11AppMask_width, TypicalX11AppMask_height,
            WhitePixel(m_disp, 0), BlackPixel(m_disp, 0),
            1);
        return status == 0;
    }

    void set_standard_properties() {
        XSizeHints *psize_hints = XAllocSizeHints();
        psize_hints->flags = PMinSize | PMaxSize;
        psize_hints->min_width = WIN_WIDTH;
        psize_hints->min_height = WIN_HEIGHT;
        psize_hints->max_width = WIN_WIDTH;
        psize_hints->max_height = WIN_HEIGHT;
        XSetStandardProperties(m_disp, m_win,
            PROGRAM_NAME, PROGRAM_NAME,
            m_icon_pixmap, m_argv, m_argc, psize_hints);
        XFree(psize_hints);

        XWMHints *hints = XAllocWMHints();
        hints->flags = IconPixmapHint | IconMaskHint;
        hints->icon_pixmap = m_icon_pixmap;
        hints->icon_mask = m_icon_mask;
        XSetWMHints(m_disp, m_win, hints);
        XFree(hints);
    } // set_standard_properties

    // show help
    void show_help() {
        printf("%s by %s\n", PROGRAM_NAME, PROGRAM_AUTHORS);
        printf("Usage: %s [options] files...\n", PROGRAM_NAME);
        printf("Options:\n");
        printf("--help          show this help\n");
        printf("--version       show version info\n");
        printf("--option arg    set optional option\n");
        printf("\n");
    } // show_help

    // show version
    void show_version(void) {
        printf("%s\n", VERSION_INFO_STRING);
        printf("\n");
    } // show_version

    bool parse_command_line(int& ret) {
        ret = 0;
        for (int i = 1; i < m_argc; ++i) {
            if (m_argv[i][0] == '-') {
                if (strcmp(m_argv[i], "--help") == 0) {
                    show_help();
                    return false;
                }
                if (strcmp(m_argv[i], "--version") == 0) {
                    show_version();
                    return false;
                }
                if (strcmp(m_argv[i], "--option") == 0) {
                    if (i + 1 < m_argc) {
                        m_option = m_argv[++i];
                        continue;
                    }
                    fprintf(stderr,
                        "%s: ERROR: option '--option' requires a parameter\n",
                        PROGRAM_NAME);
                    ret = 1;
                    return false;
                }
                fprintf(stderr, "%s: ERROR: invalid argument '%s'\n",
                    PROGRAM_NAME, m_argv[i]);
                ret = 2;
                return false;
            } else {
                m_files.push_back(m_argv[i]);
            }
        }
        return true;
    } // parse_command_line

    bool startup() {
        // open display
        m_disp = XOpenDisplay(NULL);
        if (m_disp == NULL) {
            return false;
        }

        // get the root window
        m_root_win = RootWindow(m_disp, 0);

        // create the main window
        m_win = XCreateSimpleWindow(m_disp, m_root_win,
            0, 0, WIN_WIDTH, WIN_HEIGHT, BORDER_WIDTH,
            WhitePixel(m_disp, 0),
            BlackPixel(m_disp, 0)
        );
        if (m_win == None) {
            fprintf(stderr, "%s: ERROR: XCreateSimpleWindow fails\n",
                PROGRAM_NAME);
            return false;
        }

        // set event masks
        XSelectInput(m_disp, m_win,
            ExposureMask |
            ButtonPressMask | ButtonReleaseMask | ButtonMotionMask |
            KeyPressMask | KeyReleaseMask |
            StructureNotifyMask);

        // create graphic contexts
        m_gc1 = XCreateGC(m_disp, m_win, 0, NULL);
        m_gc2 = XCreateGC(m_disp, m_win, 0, NULL);
        m_gc3 = XCreateGC(m_disp, m_win, 0, NULL);

        // set WM_DELETE_WINDOW protocol
        m_wm_delete_window = XInternAtom(m_disp, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(m_disp, m_win, &m_wm_delete_window, 1);

        // load icon pixmap
        load_icon_pixmap();

        // set standard properties
        set_standard_properties();

        // show the window
        XMapWindow(m_disp, m_win);

        return true;
    } // startup

    int run() {
        // event loop
        XEvent e;
        while (!m_quit) {
            if (XPending(m_disp)) {
                XLockDisplay(m_disp);   // multithread support
                XNextEvent(m_disp, &e);
                XUnlockDisplay(m_disp); // multithread support
                switch (e.type) {
                case Expose:
                    onExpose(&e.xexpose);
                    break;
                case ButtonPress:
                    onButtonPress(&e.xbutton);
                    break;
                case ButtonRelease:
                    onButtonRelease(&e.xbutton);
                    break;
                case MotionNotify:
                    onMotionNotify(&e.xmotion);
                    break;
                case KeyPress:
                    onKeyPress(&e.xkey);
                    break;
                case KeyRelease:
                    onKeyRelease(&e.xkey);
                    break;
                case ClientMessage:
                    onClientMessage(&e.xclient);
                    break;
                case DestroyNotify:
                    onDestroyNotify(&e.xdestroywindow);
                    break;
                case MappingNotify:
                    onMappingNotify(&e.xmapping);
                    break;
                }
            } else {
                onIdle();
            }
        }
        return 0;
    } // run

    //
    // event handlers
    //
    void onExpose(XExposeEvent *pee) {
        // get width and height
        unsigned int width, height;
        {
            Window root;
            int x, y;
            unsigned int bwidth, depth;
            XGetGeometry(m_disp, m_win, &root, &x, &y, &width, &height,
                &bwidth, &depth);
        }

        // clear window
        XClearWindow(m_disp, m_win);

        // draw lines
        XSetForeground(m_disp, m_gc1, WhitePixel(m_disp, 0));
        XSetLineAttributes(m_disp, m_gc1, 1, LineSolid, CapRound, JoinRound);
        XDrawLine(m_disp, m_win, m_gc1, 0, 0, width, height);
        XDrawLine(m_disp, m_win, m_gc1, width, 0, 0, height);

        // draw string
        Font font = XLoadFont(m_disp, "f*");
        XSetFont(m_disp, m_gc1, font);
        {
            static char text[] = PROGRAM_NAME;
            int text_len = (int)strlen(text);
            int ascent;
            {
                int direction, descent;
                XCharStruct cs;
                XQueryTextExtents(m_disp, font, text, text_len,
                    &direction, &ascent, &descent, &cs);
            }
            XDrawString(m_disp, m_win, m_gc1, 0, ascent, text, text_len);
        }
        XUnloadFont(m_disp, font);
    }
    void onButtonPress(XButtonEvent *pbe) {
        printf("pressed button: %d, x: %d, y: %d\n", pbe->button, pbe->x, pbe->y);
    }
    void onButtonRelease(XButtonEvent *pbe) {
        printf("released button: %d, x: %d, y: %d\n", pbe->button, pbe->x, pbe->y);
    }
    void onMotionNotify(XMotionEvent *pme) {
        ;
    }
    void onKeyPress(XKeyEvent *pke) {
        // See <X11/keysymdef.h>
        KeySym ks;
        ks = XLookupKeysym(pke, 0);
        switch (ks)
        {
        case XK_Shift_L:
        case XK_Shift_R:
            m_shift_pressed = true;
            puts("Shift key pressed");
            break;
        case XK_Control_L:
        case XK_Control_R:
            m_ctrl_pressed = true;
            puts("Ctrl key pressed");
            break;
        default:
            break;
        }
    }
    void onKeyRelease(XKeyEvent *pke) {
        // See <X11/keysymdef.h>
        KeySym ks;
        ks = XLookupKeysym(pke, 0);
        switch (ks)
        {
        case XK_Shift_L:
        case XK_Shift_R:
            m_shift_pressed = false;
            puts("Shift key released");
            break;
        case XK_Control_L:
        case XK_Control_R:
            m_ctrl_pressed = false;
            puts("Ctrl key released");
            break;
        default:
            break;
        }
    }
    void onMappingNotify(XMappingEvent *pme) {
        XRefreshKeyboardMapping(pme);
    }
    void onIdle() {
        ;
    }
    void onClientMessage(XClientMessageEvent *pcme) {
        if ((Atom)pcme->data.l[0] == m_wm_delete_window) {
            XDestroyWindow(m_disp, m_win);
        }
    }
    void onDestroyNotify(XDestroyWindowEvent *pdwe) {
        XSetCloseDownMode(m_disp, DestroyAll);
        m_quit = true;
    }
}; // struct X11App

//////////////////////////////////////////////////////////////////////////////
// the main function

int main(int argc, char **argv) {
    int ret;

    try {
        // X11 multithread
        XInitThreads();

        X11App app(argc, argv);

        if (app.parse_command_line(ret)) {
            if (app.startup()) {
                ret = app.run();
            } else {
                ret = 1;
            }
        }
    } catch (const std::bad_alloc&) {
        fprintf(stderr, "%s: ERROR: Out of memory\n", PROGRAM_NAME);
        ret = -1;
    }

    return ret;
} // main

//////////////////////////////////////////////////////////////////////////////
