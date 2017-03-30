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

struct platform_event
{
    MSG Message;
    UINT MessageId;
    WPARAM WParam;
    LPARAM LParam;
    LRESULT Result;
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


void
PlatformCloseWindow(platform_window* Window)
{
    if(Window)
    {
        CloseWindow(Window->Handle);
    }
}

b32
PlatformDrawWindow(platform_window* Window)
{
    return false;
}

void
PlatformHandleEvent(platform_window Window, platform_event* Event)
{
    switch(Event->MessageId)
    {
        case WM_SIZE:
        {
            OutputDebugStringA("WM_SIZE\n");
        } break;

        case WM_DESTROY:
        {
            OutputDebugStringA("WM_DESTROY\n");
            GlobalIsRunning = false;
        } break;

        case WM_CLOSE:
        {
            OutputDebugStringA("WM_CLOSE\n");
            GlobalIsRunning = false;
        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVEAPP\n");
        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window.Handle, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            LONG Width = Paint.rcPaint.right - Paint.rcPaint.left;
            LONG Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            PatBlt(DeviceContext, X, Y, Width, Height, WHITENESS);
            EndPaint(Window.Handle, &Paint);
        } break;

        default:
        {
            Event->Result = DefWindowProc(Window.Handle, Event->MessageId, Event->WParam, Event->LParam);
        } break;
    }
}

LRESULT CALLBACK
MainWindowCallback(HWND WindowHandle,
                   UINT MessageId,
                   WPARAM WParam,
                   LPARAM LParam)
{
    platform_window Window = {};
    Window.Handle = WindowHandle;
    
    platform_event Event = {};
    Event.MessageId = MessageId;
    Event.WParam = WParam;
    Event.LParam = LParam;

    PlatformHandleEvent(Window, &Event);
    
    return Event.Result;
}

platform_window
PlatformOpenWindow()
{
    platform_window Result = {};
    
    WNDCLASS WindowClass = Result.WindowClass;

    WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = GetModuleHandle(NULL); // Get the handle of exe
    //    WindowClass.hIcon;
    WindowClass.lpszClassName = "CalendarWindowClass";

    if( RegisterClass(&WindowClass) )
    {
        Result.Handle =
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
                           WindowClass.hInstance, // Get instance of exe
                           0);
    }
    else
    {
        // @Logging
    }

    Assert(Result.Handle);

    if(Result.Handle)
    {
        // @Logging
        Result = {};
    }
    
    return Result;
}

platform_event* PlatformGetNextEvent(platform_window* Window)
{
    platform_event* Result = {};
    BOOL MessageResult = GetMessage(&Result->Message, 0, 0, 0);

    if( MessageResult > 0 )
    {
        Result->MessageId = Result->Message.message;
        TranslateMessage(&Result->Message);
        DispatchMessage(&Result->Message);
    }
    
    return Result;
}

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
    platform_window Window = PlatformOpenWindow();
    
    GameMain(0, NULL, &Window);
    
    return 0;
}

#endif
