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

LRESULT CALLBACK
MainWindowCallback(HWND Window,
                   UINT Message,
                   WPARAM WParam,
                   LPARAM LParam)
{
    LRESULT Result = 0;
    
    switch(Message)
    {
        case WM_SIZE:
        {
            OutputDebugStringA("WM_SIZE\n");
        } break;

        case WM_DESTROY:
        {
            OutputDebugStringA("WM_DESTROY\n");
        } break;

        case WM_CLOSE:
        {
            PostQuitMessage(0);
            OutputDebugStringA("WM_CLOSE\n");
        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVEAPP\n");
        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            LONG Width = Paint.rcPaint.right - Paint.rcPaint.left;
            LONG Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            PatBlt(DeviceContext, X, Y, Width, Height, WHITENESS);
            EndPaint(Window, &Paint);
        } break;

        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }

    return Result;
}

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
    WNDCLASS WindowClass = {};

    WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = Instance;
    //    WindowClass.hIcon;
    WindowClass.lpszClassName = "CalendarWindowClass";

    if( RegisterClass(&WindowClass) )
    {
        HWND WindowHandle =
            CreateWindowEx(0,
                           WindowClass.lpszClassName,
                           "Calendar",
                           WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                           CW_USEDEFAULT, //x
                           CW_USEDEFAULT, //y
                           CW_USEDEFAULT, //Width
                           CW_USEDEFAULT, //Height
                           0,
                           0,
                           Instance,
                           0);
        if( WindowHandle )
        {
            MSG Message;
            for(;;)
            {
                BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
                if( MessageResult > 0 )
                {
                    TranslateMessage(&Message); //Prepares message
                    DispatchMessage(&Message);
                }
                else
                {
                    break;
                }
            }
        }
    }
    else
    {
        // @Logging
    }
    
    GameMain(0, NULL, NULL);
    
    return 0;
}

#endif
