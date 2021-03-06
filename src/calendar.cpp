#include "calendar.h"

// Global vbariable to indicate the game is running
global bool GlobalIsRunning = true;

inline b32
IsLeapYear(int Year)
{
    b32 Result = Year % 4 == 0 && (Year % 100 != 0 || Year % 400 == 0);

    return Result;
}

inline int
Floor(float f)
{
    int Result = (int) f;
    if(f < 0)
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

#define Seconds(Quantity) (Quantity * 1000)
#define Minutes(Quantity) (Seconds(60 * Quantity))
#define Hours(Quantity) (Minutes(60 * Quantity))
#define Days(Quantity) (Hours(24 * Quantity))

inline u64
DateTimeToMilliseconds(date_time dt)
{
    local_persist int DaysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    u64 Result = Days(dt.Year * 365 + IsLeapYear(dt.Year)) + Days(DaysInMonth[dt.Month]) + Days(dt.Day) + Hours(dt.Hour)
                 + Seconds(dt.Second);

    return Result;
}

inline u64
DurationOfTimeInterval(date_time Start, date_time End)
{
    Assert(DateTimeToMilliseconds(Start) <= DateTimeToMilliseconds(End));

    u64 Result = DateTimeToMilliseconds(End) - DateTimeToMilliseconds(Start);

    return Result;
}

// https://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week#Implementation-dependent_methods
week_day
WeekDayFromDate(int Day, int Month, int Year)
{
    week_day Result;

    local_persist int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    Year -= Month < 3;
    int WeekDay = (Year + Year / 4 - Year / 100 + Year / 400 + t[Month - 1] + Day) % 7;

    Result = (week_day)((WeekDay + 6) % 7);

    return Result;
}

void
PrintCalendarMonthHeader()
{
    local_persist const char* WeekDays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    for(int i = 0; i < ArrayCount(WeekDays); i++)
    {
        printf("%4s ", WeekDays[i]);
    }
    printf("\n");
}

void
PrintCalendarMonth(month Month, week_day StartingWeekDay, b32 IsLeapYear = false)
{
    int DaysInMonth = Month.Days;

    if(Month.Index == FEBRUARY && IsLeapYear)
    {
        DaysInMonth += 1;
    }

    for(int i = (StartingWeekDay + 1) % 7; i > 0; i--)
    {
        printf("[   ]");
    }

    for(int i = 0; i < Month.Days; i++)
    {
        printf("[%3d]", i + 1);
        if((i + StartingWeekDay) % 7 == SATURDAY)
        {
            printf("\n");
        }
    }

    for(int i = (Month.Days + StartingWeekDay) % 7; i < SUNDAY; i++)
    {
        printf("[   ]");
    }

    printf("\n");
}

void
PrintCalendarYear(calendar_year_node* CalendarYear)
{
    int TotalDays = 0;

    for(int MonthIndex = 0; MonthIndex < ArrayCount(CalendarYear->Months); MonthIndex++)
    {
        month* CurrentMonth = CalendarYear->Months + MonthIndex;
        if(CurrentMonth)
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
CalendarScheduleInititalize(calendar_year_node* Calendar, calendar_schedule* Schedule)
{
    Schedule->InititalCalendarYear = Calendar;
    Schedule->MinimumTimeSlotSize  = 300;
    Schedule->DefaultTimeSlotSize  = 30 * 60;

    // Reasonable upper limit on possible entries
    Schedule->EntryCount     = 365 * 24 * 60 / Schedule->MinimumTimeSlotSize;
    Schedule->FreeEntryCount = Schedule->EntryCount;
    Schedule->Entries = (calendar_schedule_entry*) malloc(Schedule->FreeEntryCount * sizeof(calendar_schedule_entry));
}

#if false
void
PrintCalendar(calendar_year_node* CalendarYear)
{
    month Month              = CalendarYear->Months[CalendarYear->CurrentMonth];
    week_day StartingWeekDay = SUNDAY;
    int DaysInMonth          = Month.Days;

    if(Month.Index == FEBRUARY && IsLeapYear(CalendarYear->Year))
    {
        DaysInMonth += 1;
    }

    for(int i = (StartingWeekDay + 1) % 7; i > 0; i--)
    {
        printf("[   ]");
    }

    for(int i = 0; i < Month.Days; i++)
    {
        printf("[%3d]", i + 1);
        if((i + StartingWeekDay) % 7 == SATURDAY)
        {
            printf("\n");
        }
    }

    for(int i = (Month.Days + StartingWeekDay) % 7; i < SUNDAY; i++)
    {
        printf("[   ]");
    }

    printf("\n");
}
#endif

void
DrawClock(platform_window* Window, u32 CenterX, u32 CenterY, u32 Radius)
{
    int thetaOffset     = 90;
    timestamp Timestamp = PlatformGetTime();
    int theta           = (Timestamp.Seconds % 60) * 6 - thetaOffset;

    PlatformDrawCircle(Window, CenterX, CenterY, Radius);

    u32 LineX1 = CenterX;
    u32 LineY1 = CenterY;
    u32 LineX2 = CenterX + Radius * cos(PI * theta / 180);
    u32 LineY2 = CenterY + Radius * sin(PI * theta / 180);

    PlatformDrawLine(Window, LineX1, LineY1, LineX2, LineY2);

    local_persist const char* ClockNumbers[] = {"12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11"};

    for(int i = 0; i < ArrayCount(ClockNumbers); i++)
    {
        int NumberTheta = i * 360 / ArrayCount(ClockNumbers) - thetaOffset;

        PlatformDrawString(Window, CenterX + Radius * cos(PI * NumberTheta / 180),
                           CenterY + Radius * sin(PI * NumberTheta / 180), (char*) ClockNumbers[i],
                           strlen(ClockNumbers[i]));
    }
}

void
DrawClockNumerical(platform_window* Window, u32 TopLeftX, u32 TopLeftY, u32 Width, u32 Height)
{
    PlatformClearArea(Window, TopLeftX, TopLeftY, Width, Height);
    // hour, minute, second only
    date_time LocalDateTime = PlatformGetLocalDateTime();
    char ClockString[8];

    sprintf(ClockString, "%d:%02d:%02d", LocalDateTime.Hour, LocalDateTime.Minute, LocalDateTime.Second);

    u32 CenterX = TopLeftX + Width / 2 - 100;
    u32 CenterY = TopLeftY + Height / 2 + 20;
    PlatformDrawRectangle(Window, TopLeftX, TopLeftY, Width, Height);
    PlatformDrawString(Window, CenterX, CenterY, ClockString, strlen(ClockString));
}

void
DrawCalendarHeader(platform_window* Window, rectangle Dimension)
{

    // u32 TopLeftX, u32 TopLeftY, u32 Width, u32 Height

    local_persist const char* WeekDays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    u32 CellWidth = Dimension.Width / ArrayCount(WeekDays);

    u32 y = Dimension.OffsetY + Dimension.Height / 2;

    for(int i = 0; i < ArrayCount(WeekDays); i++)
    {
        u32 x = i * CellWidth + Dimension.OffsetX;
        PlatformDrawString(Window, x, y, (char*) WeekDays[i], strlen(WeekDays[i]));
    }
}

void
DrawGrid(platform_window* Window, u32 OffsetX, u32 OffsetY, u32 Width, u32 Height, u32 Rows, u32 Columns)
{
    Assert(Rows > 0);
    Assert(Columns > 0);

    f32 CellWidth  = Width / (f32) Columns;
    f32 CellHeight = Height / (f32) Rows;

    for(u32 i = 0; i <= Columns; i++)
    {
        f32 ColumnOffset = i * CellWidth;
        PlatformDrawLine(Window, ColumnOffset + OffsetX, OffsetY, ColumnOffset + OffsetX, Height + OffsetY);
    }

    for(u32 i = 0; i <= Rows; i++)
    {
        f32 RowOffset = i * CellHeight;
        PlatformDrawLine(Window, OffsetX, RowOffset + OffsetY, Width + OffsetX, RowOffset + OffsetY);
    }
}

void
DrawCalendarDays(platform_window* Window, rectangle Dimension, u32 Rows, u32 Columns, month Month,
                 week_day StartingWeekDay, b32 IsLeapYear = false)
{
    int DaysInMonth = Month.Days;

    if(IsLeapYear)
    {
        DaysInMonth += 1;
    }

    for(int i = (StartingWeekDay + 1) % 7; i > 0; i--)
    {
    }

    char DayStringBuffer[3];

    u32 HorizontalDelta = Dimension.Width / (float) Columns;
    u32 VerticalDelta   = Dimension.Height / (float) Rows;
    u32 CurrentColumn   = 0;
    u32 CurrentRow      = 0;

    for(int i = 0; i < Month.Days; i++)
    {
        CurrentColumn = (i + StartingWeekDay + 1) % 7;
        u32 PositionX = Dimension.OffsetX + HorizontalDelta / 2 + CurrentColumn * HorizontalDelta;
        u32 PositionY = Dimension.OffsetY + VerticalDelta / 2 + CurrentRow * VerticalDelta;
        sprintf(DayStringBuffer, "%d", i + 1);
        PlatformDrawString(Window, PositionX, PositionY, DayStringBuffer, strlen(DayStringBuffer));

        if(CurrentColumn == SUNDAY)
        {
            CurrentRow++;
        }
    }

    for(int i = (Month.Days + StartingWeekDay) % 7; i < SUNDAY; i++)
    {
    }
}

void
DrawCalendar(platform_window* Window, rectangle Dimension, calendar_year_node* CalendarYear)
{
    month Month     = CalendarYear->Months[CalendarYear->CurrentMonth];
    int DaysInMonth = Month.Days;

    u32 NumberOfColumns = 7;
    u32 NumberOfRows    = ceil((float) (DaysInMonth / NumberOfColumns)) + 1;

    Assert(NumberOfRows > 0);

    DrawGrid(Window, Dimension.OffsetX, Dimension.OffsetY, Dimension.Width, Dimension.Height, NumberOfRows,
             NumberOfColumns);
}

void
RenderWindow(platform_window* Window, calendar_year_node* CalendarYear)
{
    int Width  = PlatformWindowWidth(Window);
    int Height = PlatformWindowHeight(Window);

    PlatformClearWindow(Window);

    DrawClock(Window, Width / 2, Height / 2, 250);

    // DrawCalendarHeader(platform_window* Window, u32 TopLeftX, u32 TopLeftY, u32 Width, u32 Height)
    rectangle CalendarHeaderDimension
        = {(u32)(Width * 0.8), (u32)(Height * 0.1), (u32)(Width * 0.125), (u32)(Height * 0.04)};
    DrawCalendarHeader(Window, CalendarHeaderDimension);

    rectangle CalendarDimension = {(u32)(Width * 0.8), (u32)(Height * 0.8), (u32)(Width * 0.1), (u32)(Height * 0.1)};
    DrawCalendar(Window, CalendarDimension, CalendarYear);
    DrawClockNumerical(Window, Width / 2 - 125, Height * 0.9, 250, 75);

    DrawCalendarDays(Window, CalendarDimension, 5, 7, CalendarYear->Months[CalendarYear->CurrentMonth], MONDAY, false);
}

int
GameMain(int argc, char* argv[], platform_window* Window)
{
    char Title[] = "Homemade Calendar";
    PlatformSetWindowTitle(Window, Title);

    calendar_year_node InitialCalendarYear = {2016,
                                              {
                                                  {JANUARY, "January", 31},
                                                  {FEBRUARY, "February", 28},
                                                  {MARCH, "March", 31},
                                                  {APRIL, "April", 30},
                                                  {MAY, "May", 31},
                                                  {JUNE, "June", 30},
                                                  {JULY, "July", 31},
                                                  {AUGUST, "August", 31},
                                                  {SEPTEMBER, "September", 30},
                                                  {OCTOBER, "October", 31},
                                                  {NOVEMBER, "November", 30},
                                                  {DECEMBER, "December", 31},
                                              },
                                              JANUARY,
                                              NULL,
                                              NULL};

    calendar_year_node* NextCalendarYear = (calendar_year_node*) malloc(sizeof(calendar_year_node));
    InitialCalendarYear.NextYear         = NextCalendarYear;
    NextCalendarYear->Year               = InitialCalendarYear.Year + 1;

    PrintCalendarYear(&InitialCalendarYear);

    month CurrentMonth = InitialCalendarYear.Months[InitialCalendarYear.CurrentMonth];

    printf("Month{%d, %s, %d}\n", CurrentMonth.Index, CurrentMonth.Name, CurrentMonth.Days);

    PrintCalendarMonthHeader();
    PrintCalendarMonth(CurrentMonth, SUNDAY);

    calendar_schedule* Schedule = (calendar_schedule*) malloc(sizeof(calendar_schedule));
    CalendarScheduleInititalize(&InitialCalendarYear, Schedule);

    calendar_schedule_entry* Entry = Schedule->Entries;
    strcpy(Entry->Title, "Example");
    Entry->StartTime.Day  = 11;
    Entry->StartTime.Hour = 3;

    Entry->EndTime.Day  = 11;
    Entry->EndTime.Hour = 11;
    Schedule->EntryCount--;

    for(u32 EntryIndex = 0; EntryIndex < Schedule->EntryCount; EntryIndex++)
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
            printf("Title: %s\nStartTime: %d\nEndTime: %d\nDuration: %lu\n", Entry->Title, Entry->StartTime.Hour,
                   Entry->EndTime.Hour, (unsigned long) Duration);
        }
    }

    timestamp PreviousTimestamp = PlatformGetTime();
    timestamp CurrentTimestamp  = PreviousTimestamp;
    f32 dtInMicroseconds        = 0.0;
    f32 MicrosecondsPerFrame    = (1 / 60.0) * 1000 * 1000;

    platform_event* Event = PlatformAllocateMemoryForEvent();

    b32 eventReceived = true;
    do
    {
        while(dtInMicroseconds <= MicrosecondsPerFrame)
        {
            eventReceived = PlatformGetNextEvent(Window, Event);
            while(eventReceived)
            {
                PlatformHandleEvent(Window, Event);
                eventReceived = PlatformGetNextEvent(Window, Event);
            }

            PreviousTimestamp = CurrentTimestamp;
            CurrentTimestamp  = PlatformGetTime();
            dtInMicroseconds += CurrentTimestamp.Microseconds - PreviousTimestamp.Microseconds;
        }
        RenderWindow(Window, &InitialCalendarYear);
        PlatformClearWindow(Window);
        dtInMicroseconds = 0;
    } while(GlobalIsRunning);

    free(Window);
    free(Event);
    free(NextCalendarYear);
    free(Schedule->Entries);
    free(Schedule);
    return 0;
}
