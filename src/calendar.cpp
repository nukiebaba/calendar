#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#if COMPILER_LLVM
#include <x86intrin.h>
#endif

#include <math.h>

#if __gnu_linux__
#include <X11/Xlib.h>
#include <sys/time.h>
#endif

#define ArrayCount(x) (int)((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#define local_persist static
#define global static

#define PI 3.14159265f

#define Assert(Expression)                                              \
{                                                                       \
    if( !(Expression) )                                                 \
    {                                                                   \
        fprintf(stderr, "%s:%s:%d: Assertion failed for (%s)\n",        \
                __FILE__, __func__, __LINE__, #Expression);             \
        exit(EXIT_FAILURE);                                             \
    }                                                                   \
}                                                                       \
 
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float real32;
typedef double real64;

typedef real32 r32;
typedef real64 r64;
typedef real32 f32;
typedef real64 f64;
    


typedef enum
{
    MONDAY,
    TUESDAY,
    WEDNESDAY,
    THURSDAY,
    FRIDAY,
    SATURDAY,
    SUNDAY
} week_day;

typedef enum
{
    NONE,
    JANUARY,
    FEBRUARY,
    MARCH,
    APRIL,
    MAY,
    JUNE,
    JULY,
    AUGUST,
    SEPTEMBER,
    OCTOBER,
    NOVEMBER,
    DECEMBER
} month_name;

//ISO 8601
struct date_time
{
    u16 Year;
    u8 Month;
    u8 Day;
    u8 Hour;
    u8 Minute;
    u8 Second;
    u8 offsetHour;
    u8 offsetMinute;
};

typedef struct month
{
    month_name Index;
    const char* Name;
    u8 Days;
} month;


global month GlobalMonthArray[12] = {

};


inline bool IsLeapYear(int Year)
{
    bool Result = Year % 4 == 0 && (Year % 100 != 0 || Year % 400 == 0);
  
    return Result;
}

typedef struct calendar_year_node
{
    u16 Year;
    month Months[12];
    month_name CurrentMonth;
    calendar_year_node* PreviousYear;
    calendar_year_node* NextYear;
} calendar_year_node;

typedef struct calendar_schedule_entry
{
    char Title[256];
    date_time StartTime;
    date_time EndTime;
    u64 Duration;
} calendar_schedule_entry;

typedef struct calendar_schedule
{
    calendar_year_node* InititalCalendarYear;
    calendar_schedule_entry* Entries;
    u64 FreeEntryCount;
    u64 EntryCount;
    u32 MinimumTimeSlotSize;
    u32 DefaultTimeSlotSize;
} calendar_schedule;

inline int
Floor(float f)
{
    int Result = (int) f; 
    if( f < 0 )
    {
        Result -= 1;
    }
    return Result;
}

void
PrintDateTime(date_time dt)
{
    printf("%04d-%02d-%02dT%02d:%02d:%02dZ", dt.Year, dt.Month, dt.Day, dt.Hour, dt.Minute, dt.Second);
}

inline u64 DateTimeToSeconds(date_time dt)
{
    u64 Result = (dt.Year * 365 + IsLeapYear(dt.Year)) * 24 * 60 * 60
        + GlobalMonthArray[dt.Month].Days * 24 * 60 * 60
        + dt.Day * 24 * 60 * 60
        + dt.Hour * 60
        + dt.Second;

    return Result;
}

inline u64
DurationOfTimeInterval(date_time Start, date_time End)
{
    Assert(DateTimeToSeconds(Start) <= DateTimeToSeconds(End));

    u64 Result = DateTimeToSeconds(End) - DateTimeToSeconds(Start);
    
    return Result;
}


//https://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week#Implementation-dependent_methods
week_day
WeekDayFromDate(int d, int m, int y)
{
    week_day Result;
  
    int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    y -= m < 3;
    int WeekDay = (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;

    Result = (week_day) ((WeekDay + 6) % 7);

    return Result;
}

void
PrintCalendarMonthHeader()
{
    local_persist const char* WeekDays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    for( int i = 0; i < ArrayCount(WeekDays); i++ )
    {
        printf("%4s ", WeekDays[i]);
    }
    printf("\n");
}

void
PrintCalendarMonth(month Month, week_day StartingWeekDay, bool IsLeapYear = false)
{

#if false
    printf("Index: %d\nMonth name: %s\nNumber of days: %d\n",
           Month->Index,
           Month->Name,
           Month->Days);

    printf("StartingWeekDay: %d\n", StartingWeekDay);
#endif

    int DaysInMonth = Month.Days;

    
    if( Month.Index == FEBRUARY && IsLeapYear )
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
}

void
PrintCalendarYear(calendar_year_node* CalendarYear)
{
    int TotalDays = 0;

    for( int MonthIndex = 0; MonthIndex < ArrayCount(CalendarYear->Months); MonthIndex++ )
    {
        month* CurrentMonth = CalendarYear->Months + MonthIndex;
        if( CurrentMonth )
	{
            week_day StartingWeekDay = WeekDayFromDate(1, CurrentMonth->Index, CalendarYear->Year);
            PrintCalendarMonthHeader();
            PrintCalendarMonth(*CurrentMonth, StartingWeekDay, IsLeapYear(CalendarYear->Year));
	  	  
            TotalDays += CurrentMonth->Days;
	}
    }
  
    Assert(TotalDays == 365);
}

void
CalendarScheduleInititalize( calendar_year_node* Calendar, calendar_schedule* Schedule )
{
    Schedule->InititalCalendarYear = Calendar;
    Schedule->MinimumTimeSlotSize = 300;
    Schedule->DefaultTimeSlotSize = 30 * 60;

    //Reasonable upper limit on possible entries
    Schedule->EntryCount = 365 * 24 * 60 / Schedule->MinimumTimeSlotSize;
    Schedule->FreeEntryCount = Schedule->EntryCount;
    Schedule->Entries = (calendar_schedule_entry*) malloc( Schedule->FreeEntryCount * sizeof (calendar_schedule_entry) );
}

void
DrawClock(Display *Display, Window Window, GC GraphicsContext, int WindowWidth, int WindowHeight)
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

void
DrawWindow(calendar_year_node* CalendarYear)
{
#if __gnu_linux__
    Display *Display = XOpenDisplay(NULL);
    Assert(Display != NULL);

    int Screen = DefaultScreen(Display);

    int DisplayWidth = DisplayWidth(Display, Screen);
    int DisplayHeight = DisplayHeight(Display, Screen);

    int WindowWidth = DisplayWidth * 0.25;
    int WindowHeight = DisplayHeight * 0.75;
    
    unsigned long BlackColor = BlackPixel(Display, Screen);
    unsigned long WhiteColor = WhitePixel(Display, Screen);

    Window Window = XCreateSimpleWindow(Display, RootWindow(Display, Screen),
                                        0, 0, WindowWidth, WindowHeight,
                                        1, BlackColor, WhiteColor);

    XSelectInput(Display, Window, ExposureMask | KeyPressMask );
    XMapWindow(Display, Window);

    
    GC GraphicsContext = XCreateGC(Display, Window, 0, NULL);

    XSetForeground(Display, GraphicsContext, BlackColor);
    XSetBackground(Display, GraphicsContext, WhiteColor);


    char *FontName = (char *) "-*-helvetica-*-r-*-*-50-*-*-*-*-*-*-*";
    XFontStruct *FontInfo = XLoadQueryFont(Display, FontName);

    Assert(FontInfo != NULL);

    XSetFont(Display, GraphicsContext, FontInfo->fid);
    
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

                // DrawClock(Display, Window, GraphicsContext, WindowWidth, WindowHeight);
                DrawCalendarHeader(Display, Window, GraphicsContext, WindowWidth, WindowHeight * 0.2);
                //DrawCalendar(Display, Window, GraphicsContext, WindowWidth, WindowHeight * 0.8, CalendarYear);

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
    
    XDestroyWindow(Display, Window);

    XCloseDisplay(Display);

#endif
}


int main(int argc, char *argv[])
{
    calendar_year_node InitialCalendarYear = {2016, {
        {JANUARY,   "January",   31},
        {FEBRUARY,  "February",  28},
        {MARCH,     "March",     31},
        {APRIL,     "April",     30},
        {MAY,       "May",       31},
        {JUNE,      "June",      30},
        {JULY,      "July",      31},
        {AUGUST,    "August",    31},
        {SEPTEMBER, "September", 30},
        {OCTOBER,   "October",   31},
        {NOVEMBER,  "November",  30},
        {DECEMBER,  "December",  31},
    }, JANUARY, NULL, NULL};
    
    calendar_year_node* NextCalendarYear = (calendar_year_node*) malloc( sizeof (calendar_year_node) );
    InitialCalendarYear.NextYear = NextCalendarYear;
    NextCalendarYear->Year = InitialCalendarYear.Year + 1;

    //PrintCalendarYear(&GlobalInitialCalendarYear);
    //PrintCalendarYear(NextCalendarYear);

    month CurrentMonth = InitialCalendarYear.Months[InitialCalendarYear.CurrentMonth];
    
    printf("Month{%d, %s, %d}\n", CurrentMonth.Index, CurrentMonth.Name, CurrentMonth.Days);

    PrintCalendarMonthHeader();
    PrintCalendarMonth(CurrentMonth, SUNDAY);

    calendar_schedule* Schedule = (calendar_schedule*) malloc( sizeof (calendar_schedule) );
    CalendarScheduleInititalize(&InitialCalendarYear, Schedule);

    calendar_schedule_entry* Entry = Schedule->Entries;
    strcpy(Entry->Title, "Example");
    Entry->StartTime.Day = 11;
    Entry->StartTime.Hour = 3;

    Entry->EndTime.Day = 11;
    Entry->EndTime.Hour = 11;
    Schedule->EntryCount--;

    for( u32 EntryIndex = 0; EntryIndex < Schedule->EntryCount; EntryIndex++ )
    {
        calendar_schedule_entry* Entry = &Schedule->Entries[EntryIndex];
        Assert(Entry);
        if(Entry->Title[0] != '\0')
        {
            PrintDateTime(Entry->StartTime);
            printf("\n");
            PrintDateTime(Entry->EndTime);
            printf("\n");
            
            u64 Duration = DurationOfTimeInterval(Entry->StartTime, Entry->EndTime);
            Assert(Duration >= Schedule->MinimumTimeSlotSize);
            printf("Title: %s\nStartTime: %d\nEndTime: %d\nDuration: %lu",
                   Entry->Title, Entry->StartTime.Hour, Entry->EndTime.Hour, (unsigned long)Duration);

	}
    }

    DrawWindow(&InitialCalendarYear);
    
    free(NextCalendarYear);
    free(Schedule->Entries);
    free(Schedule);
    
    return 0;
}
