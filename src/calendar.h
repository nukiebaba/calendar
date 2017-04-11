#if !defined(CALENDAR_H)

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if COMPILER_LLVM
#include <x86intrin.h>
#endif

#define ArrayCount(x) (int) ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))

#define internal static
#define local_persist static
#define global static

#define PI 3.14159265f

#if true // CALENDAR_DEBUG
#define Assert(Expression)                                                                                             \
    {                                                                                                                  \
        if(!(Expression))                                                                                              \
        {                                                                                                              \
            fprintf(stderr, "%s:%s:%d: Assertion failed for (%s)\n", __FILE__, __func__, __LINE__, #Expression);       \
            exit(EXIT_FAILURE);                                                                                        \
        }                                                                                                              \
    }
#else
#define Assert(Expression) ((void) 0)
#endif

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

typedef int b32;

typedef enum { MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY, SUNDAY } week_day;

typedef enum {
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

// ISO 8601
typedef struct date_time
{
    u16 Year;
    u8 Month;
    u8 Day;
    u8 Hour;
    u8 Minute;
    u8 Second;
    u8 Millisecond;
    u8 offsetHour;
    u8 offsetMinute;
} date_time;

typedef struct month
{
    month_name Index;
    const char* Name;
    u8 Days;
} month;

typedef struct calendar_year_node
{
    u16 Year;
    month Months[12];
    month_name CurrentMonth;
    struct calendar_year_node* PreviousYear;
    struct calendar_year_node* NextYear;
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

struct platform_window;
struct platform_event;
struct platform_event_result;
struct platform_timestamp;

typedef struct platform_window platform_window;
typedef struct platform_event platform_event;
typedef struct platform_event_result platform_event_result;
typedef struct platform_timestamp platform_timestamp;

void
PlatformOpenWindow(platform_window* Window);
void
PlatformCloseWindow(platform_window* Window);
void
PlatformClearWindow(platform_window* Window);

int
PlatformWindowWidth(platform_window* Window);
int
PlatformWindowHeight(platform_window* Window);

platform_event*
PlatformAllocateMemoryForEvent();
void
PlatformGetNextEvent(platform_window* Window, platform_event* Event);
platform_event_result*
PlatformHandleEvent(platform_window* Window, platform_event* Event);

date_time
PlatformGetTime();

void
PlatformDrawLine(platform_window* Window, u32 X1, u32 Y1, u32 X2, u32 Y2);
void
PlatformDrawString(platform_window* Window, u32 PosX, u32 PosY, char* String, u32 StringLength);
void
PlatformDrawCircle(platform_window* Window, int CenterX, int CenterY, int Radius);

void
DrawClock(platform_window* Window, u32 WindowWidth, u32 WindowHeight, u32 CenterX, u32 CenterY, u32 Radius);

#define CALENDAR_H
#endif
