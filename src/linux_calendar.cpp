#if defined(__linux__)

#include <X11/Xlib.h>
#include <sys/time.h>

struct platform_window
{
    Display* Display;
    int Screen;
    Window Handle;
    GC GraphicsContext;
    int Width;
    int Height;
    bool IsRunning;
};

struct platform_event
{
    XEvent Event;
};


struct platform_event_result
{
    XEvent Event;
};


#include "calendar.cpp"

void
PlatformDrawClock(platform_window Window, int Width, int Height)
{
    XPoint c = {Width * 0.5, Height * 0.5};
    int length = 500;
                
    timeval TimeValue;
    gettimeofday(&TimeValue, NULL);

    int thetaOffset = 90;
    int theta = (TimeValue.tv_sec % 60) * 6 - thetaOffset;


    int radius = length / 2;


    XPoint p = {c.x + radius * cos(PI * theta / 180),
                c.y + radius * sin(PI * theta / 180)};

    XArc arc = {c.x - length / 2, c.y - length / 2,
                length, length,
                0 * 64, 360 * 64};

    XSegment line = {c.x, c.y, p.x, p.y};
                
    XDrawArc(Window.Display, Window.Handle, Window.GraphicsContext,
             arc.x, arc.y,
             arc.width, arc.height,
             arc.angle1, arc.angle2
             );
                
    XDrawLine(Window.Display, Window.Handle, Window.GraphicsContext,
              line.x1, line.y1, line.x2, line.y2
              );

    local_persist const char* ClockNumbers[] = {"12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11"};

    
    int padding = 3;
    for(int i = 0; i < ArrayCount(ClockNumbers); i++)
    {
        int NumberTheta = i * 360 / ArrayCount(ClockNumbers) - thetaOffset;

        XDrawString(Window.Display, Window.Handle, Window.GraphicsContext,
                    Width / 2 + radius * cos(PI * NumberTheta / 180),
                    Height / 2 + radius * sin(PI * NumberTheta / 180),
                    ClockNumbers[i], strlen(ClockNumbers[i]));
    }
}

void
PlatformDrawCalendarHeader(platform_window Window, int Width, int Height)
{
    local_persist const char* WeekDays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    u32 CellWidth = Width / ArrayCount(WeekDays);

    u32 y = CellWidth / 2;
    
    for( int i = 0; i < ArrayCount(WeekDays); i++ )
    {
        u32 x = i * CellWidth + CellWidth / 2;
        XDrawString(Window.Display, Window.Handle, Window.GraphicsContext,
                    x, y,
                    WeekDays[i], strlen(WeekDays[i]));
    }
}

void
PlatformDrawCalendar(platform_window Window, u32 WindowWidth, u32 WindowHeight, calendar_year_node* CalendarYear)
{
    month Month = CalendarYear->Months[CalendarYear->CurrentMonth];
    week_day StartingWeekDay = SUNDAY;
    int DaysInMonth = Month.Days;
    
    u32 NumberOfColumns = 7;
    u32 NumberOfRows = ceil((float) DaysInMonth / NumberOfColumns);

    Assert(NumberOfRows > 0);
    
    for( u32 i = 0; i <= NumberOfRows; i++ )
    {
        u32 GridLine = i * WindowHeight / NumberOfRows;
        XDrawLine(Window.Display, Window.Handle, Window.GraphicsContext,
                  0, GridLine,
                  WindowWidth, GridLine
                  );
    }

    for( u32 i = 0; i <= NumberOfColumns; i++ )
    {
        u32 GridLine = i * WindowWidth / NumberOfColumns;
        XDrawLine(Window.Display, Window.Handle, Window.GraphicsContext,
                  GridLine, 0,
                  GridLine, WindowHeight
                  );
    }
    
}

void 
PlatformDrawGrid(platform_window* Window, u32 OffsetX, u32 OffsetY, u32 Width, u32 Height, u32 Rows, u32 Columns)
{
    Assert(Rows > 0);
    Assert(Columns > 0);
    
    f32 CellWidth = Width / (f32)Columns;
    f32 CellHeight = Height / (f32)Rows;    

    for( u32 i = 0; i <= Columns; i++ )
    {
        f32 ColumnOffset = i * CellWidth;
        XDrawLine(Window->Display, Window->Handle, Window->GraphicsContext,
                  ColumnOffset + OffsetX, OffsetY,
                  ColumnOffset + OffsetX, Height + OffsetY
                  );
    }
    
    for( u32 i = 0; i <= Rows; i++ )
    {
        f32 RowOffset = i * CellHeight;
        
        XDrawLine(Window->Display, Window->Handle, Window->GraphicsContext,
                  OffsetX, RowOffset + OffsetY,
                  Width + OffsetX, RowOffset + OffsetY
                  );
    }

}

void RenderWindow(platform_window Window)
{

    XGCValues Values;
    XGetGCValues(Window.Display, Window.GraphicsContext, GCForeground, &Values);
    if( Values.foreground == WhitePixel(Window.Display, Window.Screen) )
    {
        XSetForeground(Window.Display, Window.GraphicsContext, BlackPixel(Window.Display, Window.Screen));
    }
    else
    {
        XSetForeground(Window.Display, Window.GraphicsContext, BlackPixel(Window.Display, Window.Screen));
    }

    XClearWindow(Window.Display, Window.Handle);

    PlatformDrawClock(Window, Window.Width, Window.Height);
    PlatformDrawCalendarHeader(Window, Window.Width, Window.Height * 0.2);
    //PlatformDrawCalendar(Window, Window.Width, Window.Height * 0.8, CalendarYear);


}

platform_event
PlatformGetNextEvent(platform_window Window)
{
    platform_event Result = {};
    XNextEvent(Window.Display, &Result.Event);
    return Result;
}

platform_event_result
PlatformHandleEvent(platform_window Window, platform_event _Event)
{
    platform_event_result Result = {};
    XEvent Event = _Event.Event;
    switch(Event.type)
    {
        case MapNotify:
        {
                
            printf("MapNotify");
        } break;
            
        case Expose:
        {
            if( Event.xexpose.count > 0 )
            {
                break;
            }
                                
            XFlush(Window.Display);
        } break;
             
        case KeyPress:
        {
            if( Event.xkey.keycode == 24 || Event.xkey.keycode == 9 )
            {
                GlobalIsRunning = false;
            }

            RenderWindow(Window);
            printf("KeyPress {%d}\n", Event.xkey.keycode);
        } break;

        case KeyRelease:
        {
            printf("KeyRelease {%d}\n", Event.xkey.keycode);
        } break;

        case MotionNotify:
        {
            XMotionEvent MotionEvent = Event.xmotion;
            //printf("MotionNotify {%d, %d}\n", MotionEvent.x, MotionEvent.y);
        } break;
        
        case ResizeRequest:
        {
            XResizeRequestEvent ResizeRequestEvent = Event.xresizerequest;

#if false
            if (ResizeRequestEvent.width != Window.Width ||
                ResizeRequestEvent.height != ResizeRequestEvent.height) {
                Window.Width = ResizeRequestEvent.width;
                Window.Height = ResizeRequestEvent.height;

                RenderWindow(Window);
            }
#endif
            
            printf("ResizeRequest {%d, %d}\n", ResizeRequestEvent.width, ResizeRequestEvent.height);
        } break;

        case ConfigureNotify:
        {
            XConfigureEvent ConfigureEvent = Event.xconfigure;

            if (ConfigureEvent.width != Window.Width ||
                ConfigureEvent.height != ConfigureEvent.height) {
                Window.Width = ConfigureEvent.width;
                Window.Height = ConfigureEvent.height;

                RenderWindow(Window);
            }
            
            printf("%05lu: ConfigureNotify (%d) {Reconfigured Window: %lu, Changed Window: %lu, Width: %d, Height: %d}\n",
                   Event.xany.serial, Event.type, ConfigureEvent.event,  ConfigureEvent.window,
                   ConfigureEvent.width, ConfigureEvent.height);
        } break;
        
        case ConfigureRequest:
        {
            printf("ConfigureRequest\n");
        } break;

        default:
        {

        }
    }
    
    return Result;
}

platform_window
PlatformOpenWindow()
{
    platform_window Window = {};
    
    Window.Display = XOpenDisplay(NULL);
    Assert(Window.Display != NULL);
    
    Window.Screen = DefaultScreen(Window.Display);

    
    unsigned long BlackColor = BlackPixel(Window.Display, Window.Screen);
    unsigned long WhiteColor = WhitePixel(Window.Display, Window.Screen);

    
    int AspectRatio[2] = {16, 9};
    int DisplayHeight = DisplayHeight(Window.Display, Window.Screen);
    int DisplayWidth = DisplayWidth(Window.Display, Window.Screen);

    int AspectRatioHeight = DisplayHeight;
    int AspectRatioWidth = (AspectRatioHeight / AspectRatio[1]) * AspectRatio[0];

    printf("Display{%d,%d}, AspectRatio{%d,%d}\n", DisplayWidth, DisplayHeight, AspectRatioWidth, AspectRatioHeight);
    
    Window.Height = AspectRatioHeight;
    if( DisplayWidth < AspectRatioWidth )
    {
        Window.Width = DisplayWidth;
    }
    else
    {
        Window.Width = AspectRatioWidth;
    }

    Assert(AspectRatioWidth <= DisplayWidth);
    Assert(AspectRatioHeight == DisplayHeight);    

    Window.Width *= 0.8;
    Window.Height *= 0.8;
    
    Window.Handle = XCreateSimpleWindow(Window.Display,
                                         RootWindow(Window.Display, Window.Screen),
                                         100, 100,
                                         Window.Width, Window.Height,
                                         1, BlackColor, WhiteColor);
    Assert(Window.Handle != 0);

    
    XSelectInput(Window.Display, Window.Handle,
                 ExposureMask | KeyPressMask | KeyReleaseMask | PointerMotionMask
                 // | ResizeRedirectMask //Called by the parent window
                 | StructureNotifyMask // Window state changes
                 );
    XMapWindow(Window.Display, Window.Handle);


    Window.GraphicsContext = XCreateGC(Window.Display, Window.Handle, 0, NULL);

    XSetForeground(Window.Display, Window.GraphicsContext,
                   BlackPixel(Window.Display, Window.Screen));
    XSetBackground(Window.Display, Window.GraphicsContext, WhitePixel(Window.Display, Window.Screen));

    char *FontName = (char *) "-*-helvetica-*-r-*-*-50-*-*-*-*-*-*-*";
    XFontStruct *FontInfo = XLoadQueryFont(Window.Display, FontName);

    Assert(FontInfo != NULL);

    XSetFont(Window.Display, Window.GraphicsContext, FontInfo->fid);

    platform_window Result = Window;
    return Result;
}

void
PlatformCloseWindow(platform_window Window)
{
    XDestroyWindow(Window.Display, Window.Handle);
    XCloseDisplay(Window.Display);
}

int main(int argc, char *argv[])
{
    platform_window Window = PlatformOpenWindow();

    //Game logic
    GameMain(argc, argv, Window);
}

#endif
