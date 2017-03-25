#if !defined(CALENDAR_H)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#if COMPILER_LLVM
#include <x86intrin.h>
#endif

#define ArrayCount(x) (int)((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#define internal static
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

typedef int b32;

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

typedef struct calendar_year_node
{
    u16 Year;
    month Months[12];
    month_name CurrentMonth;
    struct calendar_year_node* PreviousYear;
    struct calendar_year_node* NextYear;
} calendar_year_node;



typedef struct platform_window platform_window;

struct platform_window* PlatformOpenWindow();
b32 PlatformDrawWindow(platform_window* Window, calendar_year_node* CalendarYear);
void PlatformCloseWindow(platform_window* Window);
void PlatformDrawCalendarHeader(platform_window* Window);

#define CALENDAR_H
#endif