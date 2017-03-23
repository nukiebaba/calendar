#if defined(_WIN32) || defined(_WIN64)

#include <windows.h>

#include "calendar.cpp"

struct platform_window
{
    HWND Handle;
    HDC DeviceContext;
    WNDCLASS WindowClass;
    int WindowWidth;
    int WindowHeight;
};

void
PlatformDrawClock(platform_window* Window)
{
    
}

void
DrawCalendarHeader(platform_window* Window)
{

}

void
DrawCalendar(platform_window* Window, calendar_year_node* CalendarYear)
{

}

void 
DrawGrid(platform_window* Window, u32 OffsetX, u32 OffsetY, u32 Width, u32 Height, u32 Rows, u32 Columns)
{

}

void
PlatformDrawWindow(platform_window* Window, calendar_year_node* CalendarYear)
{
    return;
}

platform_window*
PlatformOpenWindow()
{
    platform_window* Window = (platform_window*) malloc(sizeof platform_window);
    return Window;
}

void
PlatformCloseWindow(platform_window* Window)
{
    if(Window)
    {
        CloseWindow(Window->Handle);
    }
}

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
    /*
    LPWSTR* CommandLineArguments;
    int NumberOfArguments;

    CommandLineArguments = CommandLineToArgvW((LPCWSTR)CommandLine, &NumberOfArguments);
    if(CommandLineArguments)
    { 
        GameMain(NumberOfArguments, (char**) CommandLineArguments);
    }
    */

    return 0;
}

#endif
