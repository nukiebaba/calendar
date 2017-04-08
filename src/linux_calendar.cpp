#if defined(__linux__)

#include <X11/Xlib.h>
#include <sys/time.h>

// ICCCM 4.1.2.7. WM_PROTOCOLS Property
#define WM_PROTOCOLS 295
#define WM_TAKE_FOCUS 294
#define WM_DELETE_WINDOW 293

struct platform_window
{
    Display* Display;
    int Screen;
    Window Handle;
    Atom* Protocols;
    GC GraphicsContext;
    int Width;
    int Height;
};

struct platform_event
{
    XEvent Event;
};

struct platform_timestamp
{
    int Seconds;
    int Milliseconds;
};

struct platform_event_result
{
    XEvent Event;
};

#include "calendar.cpp"

int
main(int argc, char* argv[])
{
    platform_window* Window = PlatformOpenWindow();

    // Game logic
    GameMain(argc, argv, Window);
}

platform_event*
PlatformAllocateMemoryForEvent()
{
    platform_event* Result = (platform_event*) malloc(sizeof(platform_event));

    return Result;
}

date_time
PlatformGetTime()
{
    date_time Timestamp = {};

    timeval TimeValue;
    gettimeofday(&TimeValue, NULL);

    Timestamp.Second      = TimeValue.tv_sec;
    Timestamp.Millisecond = TimeValue.tv_usec / 1000;

    return Timestamp;
}

void
PlatformDrawLine(platform_window* Window, u32 X1, u32 Y1, u32 X2, u32 Y2)
{

    XDrawLine(Window->Display, Window->Handle, Window->GraphicsContext, X1, Y1, X2, Y2);
}

void
PlatformDrawGrid(platform_window* Window, u32 OffsetX, u32 OffsetY, u32 Width, u32 Height, u32 Rows, u32 Columns)
{
    Assert(Rows > 0);
    Assert(Columns > 0);

    f32 CellWidth  = Width / (f32) Columns;
    f32 CellHeight = Height / (f32) Rows;

    for(u32 i = 0; i <= Columns; i++)
    {
        f32 ColumnOffset = i * CellWidth;
        XDrawLine(Window->Display, Window->Handle, Window->GraphicsContext, ColumnOffset + OffsetX, OffsetY,
                  ColumnOffset + OffsetX, Height + OffsetY);
    }

    for(u32 i = 0; i <= Rows; i++)
    {
        f32 RowOffset = i * CellHeight;

        PlatformDrawLine(Window, OffsetX, RowOffset + OffsetY, Width + OffsetX, RowOffset + OffsetY);
    }
}

void
RenderWindow(platform_window* Window)
{

    XGCValues Values;
    XGetGCValues(Window->Display, Window->GraphicsContext, GCForeground, &Values);
    if(Values.foreground == WhitePixel(Window->Display, Window->Screen))
    {
        XSetForeground(Window->Display, Window->GraphicsContext, BlackPixel(Window->Display, Window->Screen));
    }
    else
    {
        XSetForeground(Window->Display, Window->GraphicsContext, BlackPixel(Window->Display, Window->Screen));
    }

    XClearWindow(Window->Display, Window->Handle);

    DrawClock(Window, Window->Width / 2, Window->Height / 2, 250);
    DrawCalendarHeader(Window, Window->Width, Window->Height * 0.2);
    // PlatformDrawCalendar(Window, Window->Width, Window->Height * 0.8, CalendarYear);
}

void
PlatformDrawString(platform_window* Window, u32 PosX, u32 PosY, char* String, u32 StringLength)
{
    XDrawString(Window->Display, Window->Handle, Window->GraphicsContext, PosX, PosY, String, StringLength);
}

void
PlatformDrawCircle(platform_window* Window, int CenterX, int CenterY, int Radius)
{
    XPoint c   = {CenterX, CenterY};
    int length = 2 * Radius;

    XArc arc = {c.x - Radius, c.y - Radius, length, length, 0 * 64, 360 * 64};

    XDrawArc(Window->Display, Window->Handle, Window->GraphicsContext, arc.x, arc.y, arc.width, arc.height, arc.angle1,
             arc.angle2);
}

void
PlatformGetNextEvent(platform_window* Window, platform_event* _Event)
{
    XNextEvent(Window->Display, &_Event->Event);
}

platform_window*
PlatformOpenWindow()
{
    platform_window* Window = (platform_window*) malloc(sizeof(platform_window));

    Window->Display = XOpenDisplay(NULL);
    Assert(Window->Display != NULL);

    Window->Screen = DefaultScreen(Window->Display);

    unsigned long BlackColor = BlackPixel(Window->Display, Window->Screen);
    unsigned long WhiteColor = WhitePixel(Window->Display, Window->Screen);

    int AspectRatio[2] = {16, 9};
    int DisplayHeight  = DisplayHeight(Window->Display, Window->Screen);
    int DisplayWidth   = DisplayWidth(Window->Display, Window->Screen);

    int AspectRatioHeight = DisplayHeight;
    int AspectRatioWidth  = (AspectRatioHeight / AspectRatio[1]) * AspectRatio[0];

    printf("Display{%d,%d}, AspectRatio{%d,%d}\n", DisplayWidth, DisplayHeight, AspectRatioWidth, AspectRatioHeight);

    Window->Height = AspectRatioHeight;
    if(DisplayWidth < AspectRatioWidth)
    {
        Window->Width = DisplayWidth;
    }
    else
    {
        Window->Width = AspectRatioWidth;
    }

    Assert(AspectRatioWidth <= DisplayWidth);
    Assert(AspectRatioHeight == DisplayHeight);

    Window->Width *= 0.8;
    Window->Height *= 0.8;

    Window->Handle = XCreateSimpleWindow(Window->Display, RootWindow(Window->Display, Window->Screen), 100, 100,
                                         Window->Width, Window->Height, 1, BlackColor, WhiteColor);
    Assert(Window->Handle != 0);

    // Window manager sends a ClientMessage to X11 for closing the window

    Atom WindowManagerProtocolDeleteWindow = XInternAtom(Window->Display, "WM_DELETE_WINDOW", True);
    Atom WindowManagerProtocols[]          = {WindowManagerProtocolDeleteWindow};
    int ProtocolResult
        = XSetWMProtocols(Window->Display, Window->Handle, WindowManagerProtocols, ArrayCount(WindowManagerProtocols));
    Assert(ProtocolResult);

    XSelectInput(Window->Display, Window->Handle,
                 ExposureMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | StructureNotifyMask);
    XMapWindow(Window->Display, Window->Handle);

    Window->GraphicsContext = XCreateGC(Window->Display, Window->Handle, 0, NULL);

    XSetForeground(Window->Display, Window->GraphicsContext, BlackPixel(Window->Display, Window->Screen));
    XSetBackground(Window->Display, Window->GraphicsContext, WhitePixel(Window->Display, Window->Screen));

    char* FontName        = (char*) "-*-helvetica-*-r-*-*-50-*-*-*-*-*-*-*";
    XFontStruct* FontInfo = XLoadQueryFont(Window->Display, FontName);

    Assert(FontInfo != NULL);

    XSetFont(Window->Display, Window->GraphicsContext, FontInfo->fid);

    platform_window* Result = Window;
    return Result;
}

platform_event_result*
PlatformHandleEvent(platform_window* Window, platform_event* _Event)
{
    XEvent Event = _Event->Event;

    switch(Event.type)
    {
        case MapNotify:
        {

            printf("MapNotify");
        }
        break;

        case Expose:
        {
            if(Event.xexpose.count > 0)
            {
                break;
            }

            XFlush(Window->Display);
        }
        break;

        case KeyPress:
        {
            if(Event.xkey.keycode == 24 || Event.xkey.keycode == 9)
            {
                GlobalIsRunning = false;
            }

            RenderWindow(Window);
            printf("KeyPress {%d}\n", Event.xkey.keycode);
        }
        break;

        case KeyRelease:
        {
            printf("KeyRelease {%d}\n", Event.xkey.keycode);
        }
        break;

        case MotionNotify:
        {
            XMotionEvent MotionEvent = Event.xmotion;
            printf("MotionNotify {%d, %d}\n", MotionEvent.x, MotionEvent.y);
        }
        break;

        case ResizeRequest:
        {
            XResizeRequestEvent ResizeRequestEvent = Event.xresizerequest;

#if false
            if (ResizeRequestEvent.width != Window->Width ||
                ResizeRequestEvent.height != ResizeRequestEvent.height) {
                Window->Width = ResizeRequestEvent.width;
                Window->Height = ResizeRequestEvent.height;

                RenderWindow(Window);
            }
#endif

            printf("ResizeRequest {%d, %d}\n", ResizeRequestEvent.width, ResizeRequestEvent.height);
        }
        break;

        case ConfigureNotify:
        {
            XConfigureEvent ConfigureEvent = Event.xconfigure;

            if(ConfigureEvent.width != Window->Width || ConfigureEvent.height != ConfigureEvent.height)
            {
                Window->Width  = ConfigureEvent.width;
                Window->Height = ConfigureEvent.height;

                RenderWindow(Window);
            }

            printf(
                "%05lu: ConfigureNotify (%d) {Reconfigured Window: %lu, Changed Window: %lu, Width: %d, Height: %d}\n",
                Event.xany.serial, Event.type, ConfigureEvent.event, ConfigureEvent.window, ConfigureEvent.width,
                ConfigureEvent.height);
        }
        break;

        // Window manager will send ClientMessage when client requests window deletion
        case ClientMessage:
        {

            XClientMessageEvent ClientMessageEvent = Event.xclient;

            char* AtomName = XGetAtomName(Window->Display, ClientMessageEvent.message_type);

            printf("ClientMessage {Message Type: %lu, Format: %d, MessageName: %s}\n", ClientMessageEvent.message_type,
                   ClientMessageEvent.format, AtomName);

            if(ClientMessageEvent.message_type == WM_PROTOCOLS)
            {
                // Format is 32 bits, hence using data.l
                Atom Protocol      = ClientMessageEvent.data.l[0];
                char* ProtocolName = XGetAtomName(Window->Display, Protocol);
                if(strcmp(ProtocolName, "WM_PROTOCOLS"))
                {
                    GlobalIsRunning = false;
                }
                printf("ProtocolMessage {Protocl: %lu, ProtocolName: %s}\n", Protocol, ProtocolName);
            }
        }

        default:
        {
            XAnyEvent AnyEvent = Event.xany;
        }
        break;
    }

    return NULL;
}

void
PlatformCloseWindow(platform_window Window)
{
    XDestroyWindow(Window.Display, Window.Handle);
    XCloseDisplay(Window.Display);
}

#endif
