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

#define Assert(Expression)                                      \
{                                                               \
    if( !(Expression) )                                         \
    {                                                           \
        fprintf(stderr, "%s: %s assert failed on line %d\n",    \
                __FILE__, __func__, __LINE__);                  \
        exit(EXIT_FAILURE);                                     \
    }                                                           \
}                                                               \
 
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

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
    u8 Index;
    const char* Name;
    u8 Days;
  
} month;


inline bool isLeapYear(int Year)
{
    bool Result = Year % 4 == 0 && (Year % 100 != 0 || Year % 400 == 0);
  
    return Result;
}

typedef struct calendar_year_node
{
    u16 Year;
    month* Months;
    u8 MonthCount;
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

global month GlobalMonthArray[12] = {
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
};

static calendar_year_node GlobalInitialCalendarYear = {2016, GlobalMonthArray, ArrayCount(GlobalMonthArray), NULL, NULL};

inline int Floor(float f)
{
    int Result = (int) f; 
    if( f < 0 )
    {
        Result -= 1;
    }
    return Result;
}

void PrintDateTime(date_time dt)
{
    printf("%04d-%02d-%02dT%02d:%02d:%02dZ", dt.Year, dt.Month, dt.Day, dt.Hour, dt.Minute, dt.Second);
}

inline u64 DateTimeToSeconds(date_time dt)
{
    u64 Result = (dt.Year * 365 + isLeapYear(dt.Year)) * 24 * 60 * 60
        + GlobalMonthArray[dt.Month].Days * 24 * 60 * 60
        + dt.Day * 24 * 60 * 60
        + dt.Hour * 60
        + dt.Second;

    return Result;
}

inline u64 DurationOfTimeInterval(date_time Start, date_time End)
{
    Assert(DateTimeToSeconds(Start) <= DateTimeToSeconds(End));

    u64 Result = DateTimeToSeconds(End) - DateTimeToSeconds(Start);
    
    return Result;
}


//https://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week#Implementation-dependent_methods
week_day WeekDayFromDate(int d, int m, int y)
{
    week_day Result;
  
    int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    y -= m < 3;
    int WeekDay = (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;

    Result = (week_day) ((WeekDay + 6) % 7);

    return Result;
}

void PrintCalendarMonthHeader()
{
    local_persist const char* WeekDays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    for( int i = 0; i < ArrayCount(WeekDays); i++ )
    {
        printf("%4s ", WeekDays[i]);
    }
    printf("\n");
}

void PrintCalendarMonth(month* Month, week_day StartingWeekDay, bool IsLeapYear = false)
{

#if false
    printf("Index: %d\nMonth name: %s\nNumber of days: %d\n",
           Month->Index,
           Month->Name,
           Month->Days);

    printf("StartingWeekDay: %d\n", StartingWeekDay);
#endif
  
    int DaysInMonth = Month->Days;
    if( Month->Index == FEBRUARY && IsLeapYear )
    {
        DaysInMonth += 1;
    }
  
    for( int i = (StartingWeekDay + 1) % 7; i > 0; i--)
    {
        printf("[   ]");
    }
  
    for( int i = 0; i < Month->Days; i++)
    {
        printf("[%3d]", i+1);
        if( (i + StartingWeekDay) % 7 == SATURDAY )
	{
            printf("\n");
	}
    }

    for( int i = (Month->Days + StartingWeekDay) % 7; i < SUNDAY; i++)
    {
        printf("[   ]");
    }
  
    printf("\n");
}

void PrintCalendarYear(calendar_year_node* CalendarYear)
{
    int TotalDays = 0;

    for( int MonthIndex = 0; MonthIndex < CalendarYear->MonthCount; MonthIndex++ )
    {
        month* CurrentMonth = CalendarYear->Months + MonthIndex;
        if( CurrentMonth )
	{
            week_day WeekDay = WeekDayFromDate(1, CurrentMonth->Index, CalendarYear->Year);
            PrintCalendarMonthHeader();
            PrintCalendarMonth(CurrentMonth, WeekDay, isLeapYear(CalendarYear->Year));
	  	  
            TotalDays += CurrentMonth->Days;
	}
    }
  
    Assert(TotalDays == 365);
}

void CalendarScheduleInititalize( calendar_year_node* Calendar, calendar_schedule* Schedule )
{
    Schedule->InititalCalendarYear = Calendar;
    Schedule->MinimumTimeSlotSize = 300;
    Schedule->DefaultTimeSlotSize = 30 * 60;

    //Reasonable upper limit on possible entries
    Schedule->EntryCount = 365 * 24 * 60 / Schedule->MinimumTimeSlotSize;
    Schedule->FreeEntryCount = Schedule->EntryCount;
    Schedule->Entries = (calendar_schedule_entry*) malloc( Schedule->FreeEntryCount * sizeof (calendar_schedule_entry) );
}




void PrintCalender()
{
    calendar_year_node* NextCalendarYear = (calendar_year_node*) malloc( sizeof (calendar_year_node) );
    GlobalInitialCalendarYear.NextYear = NextCalendarYear;
    NextCalendarYear->Year = GlobalInitialCalendarYear.Year + 1;
    NextCalendarYear->Months = GlobalMonthArray;
    NextCalendarYear->MonthCount = ArrayCount(GlobalMonthArray);

    //PrintCalendarYear(&GlobalInitialCalendarYear);
    //PrintCalendarYear(NextCalendarYear);
  
    PrintCalendarMonthHeader();
    PrintCalendarMonth(&GlobalMonthArray[AUGUST], SUNDAY);

    calendar_schedule* Schedule = (calendar_schedule*) malloc( sizeof (calendar_schedule) );
    CalendarScheduleInititalize(&GlobalInitialCalendarYear, Schedule);

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
  
    free(NextCalendarYear);
    free(Schedule->Entries);
    free(Schedule);
}


int main(int argc, char *argv[])
{
    PrintCalender();
    return 0;
}

#if __gnu_linux__
void drawWindow()
{

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
                                
                XDrawLine(Display, Window, GraphicsContext, 10, 60, 180, 20);
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

                timeval TimeValue;
                gettimeofday(&TimeValue, NULL);
               
                long theta = (TimeValue.tv_sec % 60);

                int length = 500;

                XPoint c = {WindowWidth * 0.5, WindowHeight * 0.5};
                
                float dx = sin(theta) * (length / 2.0);
                float dy = cos(theta) * (length / 2.0);

                XSegment line = {c.x, c.y, c.x + dx, c.y + dy};

                XClearWindow(Display,Window);

                XDrawRectangle(Display, Window, GraphicsContext,
                               c.x - length / 2, c.y - length / 2,
                               length, length);

                XArc arc = {c.x - length / 2, c.y - length / 2,
                            length, length,
                            0 * 64, 360 * 64};
                
                XDrawArc(Display, Window, GraphicsContext,
                         arc.x, arc.y,
                         arc.width, arc.height,
                         arc.angle1, arc.angle2
                         );
                
                XDrawLine(Display, Window, GraphicsContext,
                          line.x1, line.y1, line.x2, line.y2
                          );

                
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
}
#endif
