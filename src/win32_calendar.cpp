#if defined(_WIN32) || defined(_WIN64)

#include "calendar.cpp"

#include <windows.h>

struct platform_window
{
    HWND Handle;
    HDC DeviceContext;
    WNDCLASS WindowClass;
    int Width;
    int Height;
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

b32
PlatformDrawWindow(platform_window* Window, calendar_year_node* CalendarYear)
{
    return false;
}

platform_window*
PlatformOpenWindow()
{
    platform_window* Window = (platform_window*) malloc(sizeof platform_window);

    //Window->Handle = 

    
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
    WNDCLASS WindowClass = {};

    

    //GameMain(0, NULL, NULL);
    
    return 0;
}

#endif
