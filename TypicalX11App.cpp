#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>
#include <cstdio>
#include <cstring>
#include "TypicalX11App.h"
#include "TypicalX11App.xpm"
using namespace std;

// number of graphic contexts
#define NUM_GC      3

// proram name
#define PROGNAME    "TypicalX11App"
static char progname[] = PROGNAME;

// size of window
#define WIN_WIDTH   640
#define WIN_HEIGHT  400

// border width
#define BORDER_WIDTH 1

// the application
struct X11App {
    int             m_argc;             // number of command line paramters
    char **         m_argv;             // command line paramters

    Display *       m_disp;             // X11 display
    Window          m_root_win;         // the root window
    Window          m_win;              // the main window
    GC              m_gcs[NUM_GC];      // graphic contexts
    bool            m_quit;             // quit flag
    Atom            m_wm_delete_window; // WM_DELETE_WINDOW atom

    bool            m_shift_pressed;    // Is [Shift] key pressed?
    bool            m_ctrl_pressed;     // Is [Ctrl] key pressed?

    Pixmap          m_icon_pixmap;      // icon pixmap

    X11App(int argc, char **argv) : m_argc(argc), m_argv(argv) {
        m_quit = false;
        m_shift_pressed = false;
        m_ctrl_pressed = false;
    }

    ~X11App() {
        // free icon pixmap
        if (m_icon_pixmap != None) {
            XFreePixmap(m_disp, m_icon_pixmap);
        }
        // free graphic contexts
        for (int i = 0; i < NUM_GC; ++i) {
            XFreeGC(m_disp, m_gcs[i]);
        }
        // close display
        XCloseDisplay(m_disp);
    }

    bool load_icon_pixmap() {
        m_icon_pixmap = None;
        int status = XpmCreatePixmapFromData(
            m_disp, m_root_win, (char **)TypicalX11App_xpm,
            &m_icon_pixmap,
            NULL, NULL);
        return status == 0;
    }

    void set_standard_properties() {
        XSizeHints *psize_hints = XAllocSizeHints();
        psize_hints->flags = PMinSize | PMaxSize;
        psize_hints->min_width = WIN_WIDTH;
        psize_hints->min_height = WIN_HEIGHT;
        psize_hints->max_width = WIN_WIDTH;
        psize_hints->max_height = WIN_HEIGHT;
        XSetStandardProperties(m_disp, m_win, progname, progname,
            m_icon_pixmap, m_argv, m_argc, psize_hints);
        XFree(psize_hints);
    } // set_standard_properties

    bool startup() {
        // open display
        m_disp = XOpenDisplay(NULL);
        if (m_disp != NULL) {
            // get the root window
            m_root_win = RootWindow(m_disp, 0);

            // create the main window
            m_win = XCreateSimpleWindow(m_disp, m_root_win,
                0, 0, WIN_WIDTH, WIN_HEIGHT, BORDER_WIDTH,
                WhitePixel(m_disp, 0),
                BlackPixel(m_disp, 0)
            );
            if (m_win != 0) {
                // set event masks
                XSelectInput(m_disp, m_win,
                    ExposureMask |
                    ButtonPressMask | ButtonReleaseMask | ButtonMotionMask |
                    KeyPressMask | KeyReleaseMask |
                    StructureNotifyMask);

                // create graphic contexts
                for (int i = 0; i < NUM_GC; ++i) {
                    m_gcs[i] = XCreateGC(m_disp, m_win, 0, 0);
                }

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
            } else {
                fprintf(stderr, PROGNAME ": ERROR: XCreateSimpleWindow fails\n");
            }
        }
        return false;
    }

    int run() {
        // event loop
        XEvent e;
        while (!m_quit) {
            if (XPending(m_disp)) {
                XNextEvent(m_disp, &e);
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
    }

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
        XSetForeground(m_disp, m_gcs[0], WhitePixel(m_disp, 0));
        XSetLineAttributes(m_disp, m_gcs[0], 1, LineSolid, CapRound, JoinRound);
        XDrawLine(m_disp, m_win, m_gcs[0], 0, 0, width, height);
        XDrawLine(m_disp, m_win, m_gcs[0], width, 0, 0, height);

        // draw string
        Font font = XLoadFont(m_disp, "f*");
        XSetFont(m_disp, m_gcs[0], font);
        {
            static char text[] = PROGNAME;
            int text_len = (int)strlen(text);
            int ascent;
            {
                int direction, descent;
                XCharStruct cs;
                XQueryTextExtents(m_disp, font, text, text_len,
                    &direction, &ascent, &descent, &cs);
            }
            XDrawString(m_disp, m_win, m_gcs[0], 0, ascent, text, text_len);
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
};

// the main function
int main(int argc, char **argv) {
    X11App app(argc, argv);
    if (!app.startup()) {
        return 1;
    }
    return app.run();
}
