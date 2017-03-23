#if defined(__linux__)


#include <X11/Xlib.h>
#include <sys/time.h>

#include "calendar.cpp"

struct platform_window
{
    Display* Display;
    Window Window;
    GC GraphicsContext;
    int WindowWidth;
    int WindowHeight;
};

void
PlatformDrawClock(Display *Display, Window Window, GC GraphicsContext, int WindowWidth, int WindowHeight)
{
    XPoint c = {WindowWidth * 0.5, WindowHeight * 0.5};
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
                
    XDrawArc(Display, Window, GraphicsContext,
             arc.x, arc.y,
             arc.width, arc.height,
             arc.angle1, arc.angle2
             );
                
    XDrawLine(Display, Window, GraphicsContext,
              line.x1, line.y1, line.x2, line.y2
              );

    local_persist const char* ClockNumbers[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"};

    
    int padding = 3;
    for(int i = 1; i <= 12; i++)
    {
        int theta = i * 360 / 12 - thetaOffset;

        XDrawString(Display, Window, GraphicsContext,
                    WindowWidth / 2 + radius * cos(PI * theta / 180),
                    WindowHeight / 2 + radius * sin(PI * theta / 180),
                    ClockNumbers[i], strlen(ClockNumbers[i]));
    }
}

void
DrawCalendarHeader(Display* Display, Window Window, GC GraphicsContext, int WindowWidth, int WindowHeight)
{
    local_persist const char* WeekDays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    u32 CellWidth = WindowWidth / ArrayCount(WeekDays);

    u32 y = CellWidth / 2;
    
    for( int i = 0; i < ArrayCount(WeekDays); i++ )
    {
        u32 x = i * CellWidth + CellWidth / 2;
        XDrawString(Display, Window, GraphicsContext,
                    x, y,
                    WeekDays[i], strlen(WeekDays[i]));
    }
}

void
DrawCalendar(Display* Display, Window Window, GC GraphicsContext, u32 WindowWidth, u32 WindowHeight, calendar_year_node* CalendarYear)
{
    month Month = CalendarYear->Months[CalendarYear->CurrentMonth];
    week_day StartingWeekDay = SUNDAY;
    int DaysInMonth = Month.Days;
    
    if( Month.Index == FEBRUARY && IsLeapYear(CalendarYear->Year) )
    {
        DaysInMonth += 1;
    }
    
    for( int i = (StartingWeekDay + 1) % 7; i > 0; i--)
    {
        printf("[   ]");
    }
  
    for( int i = 0; i < Month.Days; i++)
    {
        printf("[%3d]", i+1);
        if( (i + StartingWeekDay) % 7 == SATURDAY )
	{
            printf("\n");
	}
    }

    for( int i = (Month.Days + StartingWeekDay) % 7; i < SUNDAY; i++)
    {
        printf("[   ]");
    }
  
    printf("\n");

    u32 NumberOfColumns = 7;
    u32 NumberOfRows = ceil((float) DaysInMonth / NumberOfColumns);

    Assert(NumberOfRows > 0);
    
    for( u32 i = 0; i <= NumberOfRows; i++ )
    {
        u32 GridLine = i * WindowHeight / NumberOfRows;
        XDrawLine(Display, Window, GraphicsContext,
                  0, GridLine,
                  WindowWidth, GridLine
                  );
    }

    for( u32 i = 0; i <= NumberOfColumns; i++ )
    {
        u32 GridLine = i * WindowWidth / NumberOfColumns;
        XDrawLine(Display, Window, GraphicsContext,
                  GridLine, 0,
                  GridLine, WindowHeight
                  );
    }
    
}

void 
DrawGrid(Display* Display, Window Window, GC GraphicsContext, u32 OffsetX, u32 OffsetY, u32 Width, u32 Height, u32 Rows, u32 Columns)
{
    Assert(Rows > 0);
    Assert(Columns > 0);
    
    f32 CellWidth = Width / (f32)Columns;
    f32 CellHeight = Height / (f32)Rows;    

    for( u32 i = 0; i <= Columns; i++ )
    {
        f32 ColumnOffset = i * CellWidth;
        XDrawLine(Display, Window, GraphicsContext,
                  ColumnOffset + OffsetX, OffsetY,
                  ColumnOffset + OffsetX, Height + OffsetY
                  );
    }
    
    for( u32 i = 0; i <= Rows; i++ )
    {
        f32 RowOffset = i * CellHeight;
        
        XDrawLine(Display, Window, GraphicsContext,
                  OffsetX, RowOffset + OffsetY,
                  Width + OffsetX, RowOffset + OffsetY
                  );
    }

}

void *
PlatformDrawWindow(calendar_year_node* CalendarYear)
{
   
    bool IsRunning = true;
    while(IsRunning)
    {
        XEvent Event;    
        XNextEvent(Display, &Event);
        switch(Event.type)
        {
            case MapNotify:
            {
                
                printf("%d", Event.type);
            } break;
            
            case Expose:
            {
                if( Event.xexpose.count > 0 )
                {
                    break;
                }
                                
                XFlush(Display);
            } break;
             
            case KeyPress:
            {
                if( Event.xkey.keycode == 24 || Event.xkey.keycode == 9 )
                {
                    IsRunning = false;
                }

                XGCValues Values;
                XGetGCValues(Display, GraphicsContext, GCForeground, &Values);
                if( Values.foreground == WhiteColor )
                {
                    XSetForeground(Display, GraphicsContext, BlackColor);
                }
                else
                {
                    XSetForeground(Display, GraphicsContext, BlackColor);
                }

                XClearWindow(Display, Window);

                DrawClock(Display, Window, GraphicsContext, WindowWidth, WindowHeight);
                DrawCalendarHeader(Display, Window, GraphicsContext, WindowWidth, WindowHeight * 0.2);
                DrawCalendar(Display, Window, GraphicsContext, WindowWidth, WindowHeight * 0.8, CalendarYear);

                DrawGrid(Display, Window, GraphicsContext,
                         WindowWidth * 0.2, WindowHeight * 0.2,
                         WindowWidth * 0.6, WindowHeight * 0.6,
                         5, 7);
                
                XFlush(Display);
            } break;
            
            case ClientMessage:
            {
                
            } break;

            default:
            {

            } break;
        }
    }
}

Display*
PlatformOpenDisplay()
{
    Display *Display = XOpenDisplay(NULL);
    Assert(Display != NULL);
    return Display;
}

void
PlatformCloseDisplay(Display* Display)
{
    XCloseDisplay(Display);
}

platform_window
PlatformOpenWindow(Display* Display)
{
    int Screen = DefaultScreen(Display);

    Window Window = XCreateSimpleWindow(Display, RootWindow(Display, Screen),
                                        0, 0, WindowWidth, WindowHeight,
                                        1, BlackColor, WhiteColor);
    return Window;
}

void
PlatformCloseWindow(Display* Display, Window Window)
{
    XDestroyWindow(Display, Window);
}

int main(int argc, char *argv[])
{


    int DisplayWidth = DisplayWidth(Display, Screen);
    int DisplayHeight = DisplayHeight(Display, Screen);

    int WindowWidth = DisplayWidth * 0.25;
    int WindowHeight = DisplayHeight * 0.75;
    
    unsigned long BlackColor = BlackPixel(Display, Screen);
    unsigned long WhiteColor = WhitePixel(Display, Screen);


    XSelectInput(Display, Window, ExposureMask | KeyPressMask );
    XMapWindow(Display, Window);

    
    GC GraphicsContext = XCreateGC(Display, Window, 0, NULL);

    XSetForeground(Display, GraphicsContext, BlackColor);
    XSetBackground(Display, GraphicsContext, WhiteColor);


    char *FontName = (char *) "-*-helvetica-*-r-*-*-50-*-*-*-*-*-*-*";
    XFontStruct *FontInfo = XLoadQueryFont(Display, FontName);

    Assert(FontInfo != NULL);

    XSetFont(Display, GraphicsContext, FontInfo->fid);
    
    //Game logic
    GameMain(argc, argv);
        
    PlatformCloseWindow(Display, Window);

    PlatformCloseDisplay(Display);
}

#endif
