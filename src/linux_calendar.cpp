#if defined(__linux__)

#include "calendar.cpp"

#include <X11/Xlib.h>
#include <sys/time.h>
#include <time.h>

struct platform_window
{
    Display* Display;
    int Screen;
    Window Handle;
    GC GraphicsContext;
    char Title[1024];
    int Width;
    int Height;
};

struct platform_event
{
    XEvent Event;
};

struct platform_event_result
{
    XEvent Event;
};

int
main(int argc, char* argv[])
{
#if CALENDAR_DEBUG
    _Xdebug = true;
#endif

    platform_window* Window = PlatformOpenWindow((char*) "Linux Calendar", 800, 450);

    // Game logic
    GameMain(argc, argv, Window);

    return 0;
}

int
PlatformWindowWidth(platform_window* Window)
{
    return Window->Width;
}

int
PlatformWindowHeight(platform_window* Window)
{
    return Window->Height;
}

void
PlatformFlushWindow(platform_window* Window)
{
    XFlush(Window->Display);
}

platform_event*
PlatformAllocateMemoryForEvent()
{
    platform_event* Result = (platform_event*) malloc(sizeof(platform_event));

    return Result;
}

timestamp
PlatformGetTime()
{
    timestamp Timestamp = {};

    timeval TimeValue;

    i32 Result = gettimeofday(&TimeValue, NULL);

    Assert(Result != -1);

    Timestamp.Seconds      = TimeValue.tv_sec;
    Timestamp.Microseconds = TimeValue.tv_usec;

    return Timestamp;
}

date_time
PlatformGetLocalDateTime()
{
    date_time Result         = {};
    time_t CalendarTime      = time(NULL);
    struct tm* LocalDateTime = localtime(&CalendarTime);

    Result.Hour   = LocalDateTime->tm_hour;
    Result.Minute = LocalDateTime->tm_min;
    Result.Second = LocalDateTime->tm_sec;

    return Result;
}

void
PlatformDrawLine(platform_window* Window, u32 X1, u32 Y1, u32 X2, u32 Y2)
{
    XDrawLine(Window->Display, Window->Handle, Window->GraphicsContext, X1, Y1, X2, Y2);
}

void
PlatformDrawString(platform_window* Window, u32 PosX, u32 PosY, char* String, u32 StringLength)
{
    XDrawString(Window->Display, Window->Handle, Window->GraphicsContext, PosX, PosY, String, StringLength);
}

void
PlatformDrawRectangle(platform_window* Window, u32 TopLeftX, u32 TopLeftY, u32 Width, u32 Height)
{
    XDrawRectangle(Window->Display, Window->Handle, Window->GraphicsContext, TopLeftX, TopLeftY, Width, Height);
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
PlatformClearArea(platform_window* Window, u32 OffsetX, u32 OffsetY, u32 Width, u32 Height)
{
    XClearArea(Window->Display, Window->Handle, OffsetX, OffsetY, Width, Height, true);
}

void
PlatformClearWindow(platform_window* Window)
{
    XSync(Window->Display, false);
    XClearWindow(Window->Display, Window->Handle);
}

b32
PlatformGetNextEvent(platform_window* Window, platform_event* _Event)
{
    XEvent Event;
    int EventsQueued = XEventsQueued(Window->Display, QueuedAfterReading);
    if(EventsQueued > 0)
    {
        XNextEvent(Window->Display, &Event);
        _Event->Event = Event;
        return true;
    }
    return false;
}

platform_window*
PlatformOpenWindow(char* Title, u32 Width, u32 Height)
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

    int ActualCount   = 0;
    char* FontPattern = (char*) "-misc-fixed-medium-r-normal-*-50-*-*-*-*-*-*-*";
    int MaxNames      = 100;
    char** FontList   = XListFonts(Window->Display, FontPattern, MaxNames, &ActualCount);

#if false
    for(int i = 0; i < ActualCount; i++)
    {
        printf("%d. Font name: %s\n", i, FontList[i]);
    }
#endif

    Assert(ActualCount > 0);

    char* FontName        = FontList[0];
    XFontStruct* FontInfo = XLoadQueryFont(Window->Display, FontName);

    Assert(FontInfo != NULL);

    XSetFont(Window->Display, Window->GraphicsContext, FontInfo->fid);

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

    return Window;
}

void
PlatformSetWindowTitle(platform_window* Window, char* Title)
{
    XStoreName(Window->Display, Window->Handle, Title);
}

b32
PlatformHandleEvent(platform_window* Window, platform_event* _Event)
{
    // LASTEvent identifies number of XEvent types
    const char* GlobalXEventTypeString[LASTEvent] = {
        "Error",          "Reply",          "KeyPress",         "KeyRelease",
        "ButtonPress",    "ButtonRelease",  "MotionNotify",     "EnterNotify",
        "LeaveNotify",    "FocusIn",        "FocusOut",         "KeymapNotify",
        "Expose",         "GraphicsExpose", "NoExpose",         "VisibilityNotify",
        "CreateNotify",   "DestroyNotify",  "UnmapNotify",      "MapNotify",
        "MapRequest",     "ReparentNotify", "ConfigureNotify",  "ConfigureRequest",
        "GravityNotify",  "ResizeRequest",  "CirculateNotify",  "CirculateRequest",
        "PropertyNotify", "SelectionClear", "SelectionRequest", "SelectionNotify",
        "ColormapNotify", "ClientMessage",  "MappingNotify",    "GenericEvent",
    };

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
            XExposeEvent ExposeEvent = Event.xexpose;
            printf("ExposeEvent {x: %d, y: %d, Width: %d, Height: %d, Count: %d, send_event: %s}\n", ExposeEvent.x,
                   ExposeEvent.y, ExposeEvent.width, ExposeEvent.height, ExposeEvent.count,
                   ExposeEvent.send_event ? "true" : "false");

            if(ExposeEvent.count > 0)
            {
                break;
            }
        }
        break;

        case KeyPress:
        {
            if(Event.xkey.keycode == 24 || Event.xkey.keycode == 9)
            {
                GlobalIsRunning = false;
            }

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

        case ConfigureNotify:
        {
            XConfigureEvent ConfigureEvent = Event.xconfigure;

            if(ConfigureEvent.width != Window->Width || ConfigureEvent.height != ConfigureEvent.height)
            {
                Window->Width  = ConfigureEvent.width;
                Window->Height = ConfigureEvent.height;
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

            printf("ClientMessage {Message Type: %lu, Format: %d, MessageName: %s}\n", ClientMessageEvent.message_type,
                   ClientMessageEvent.format, XGetAtomName(Window->Display, ClientMessageEvent.message_type));

            char* MessageTypeName = XGetAtomName(Window->Display, ClientMessageEvent.message_type);

            // WM Protocol Message
            if(strcmp(MessageTypeName, "WM_PROTOCOLS") == 0)
            {
                // Format is 32 bits, hence using data.l
                Atom Protocol  = ClientMessageEvent.data.l[0];
                Atom Timestamp = ClientMessageEvent.data.l[1];

                char* ProtocolName = XGetAtomName(Window->Display, Protocol);

                printf("WMProtocolMessage {Protocol: %lu, ProtocolMessageName: %s, Timestamp: %lu}\n", Protocol,
                       ProtocolName, Timestamp);

                if(strcmp(ProtocolName, "WM_DELETE_WINDOW") == 0)
                {
                    GlobalIsRunning = false;
                }
            }
        }
        break;

        default:
        {

            XAnyEvent AnyEvent = Event.xany;

            printf("Unhandled event {Type: %d, Type Name: %s, send_event: %d, Display: %p}\n", AnyEvent.type,
                   GlobalXEventTypeString[AnyEvent.type], AnyEvent.send_event, AnyEvent.display);

            return false;
        }
    }

    return true;
}

#endif
