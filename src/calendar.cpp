#include "calendar.h"

inline b32 IsLeapYear(int Year)
{
    b32 Result = Year % 4 == 0 && (Year % 100 != 0 || Year % 400 == 0);
  
    return Result;
}

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
    local_persist int DaysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    u64 Result = (dt.Year * 365 + IsLeapYear(dt.Year)) * 24 * 60 * 60
        + DaysInMonth[dt.Month] * 24 * 60 * 60
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
  
    local_persist int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
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
PrintCalendarMonth(month Month, week_day StartingWeekDay, b32 IsLeapYear = false)
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
PrintCalendar(calendar_year_node* CalendarYear)
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
}


int GameMain(int argc, char *argv[], platform_window* Window)
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

    //PrintCalendarYear(&InitialCalendarYear);
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

    b32 IsRunning = true;
    while(IsRunning)
    {
        IsRunning = PlatformDrawWindow(Window, &InitialCalendarYear);
    }
    
    free(NextCalendarYear);
    free(Schedule->Entries);
    free(Schedule);
    return 0;
}
