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
	BOOL fromCallback;
	MSG Message;
	UINT MessageId;
	WPARAM WParam;
	LPARAM LParam;
};

struct platform_event_result
{
	LRESULT MessageResult;
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
PlatformCloseWindow(platform_window Window)
{
    if(Window.Handle)
    {
        CloseWindow(Window.Handle);
    }
}

#define WIN32_MESSAGE_RESULT_ERROR -1
platform_event PlatformGetNextEvent(platform_window Window)
{
    platform_event Result = {};
    BOOL MessageResult = GetMessage(&Result.Message, 0, 0, 0);

    Assert(MessageResult != WIN32_MESSAGE_RESULT_ERROR);
    
    if( MessageResult > 0 )
    {
        Result.MessageId = Result.Message.message;
        TranslateMessage(&Result.Message);
        DispatchMessage(&Result.Message);
    }
    
    return Result;
}

platform_event_result
PlatformHandleEvent(platform_window Window, platform_event Event)
{
    platform_event_result Result = {};

    //NOTE: If this is not being called from the Windows callback, ignore it and early-out.
    if( !Event.fromCallback )
    {
        return Result;
    }
    
    switch(Event.MessageId)
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
            Result.MessageResult = DefWindowProc(Window.Handle, Event.MessageId, Event.WParam, Event.LParam);
        } break;
    }
    
    return Result;
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
    Event.fromCallback = true;
    Event.MessageId = MessageId;
    Event.WParam = WParam;
    Event.LParam = LParam;

    platform_event_result Result = PlatformHandleEvent(Window, Event);

    return Result.MessageResult;
}

platform_window
PlatformOpenWindow()
{
    platform_window Result = {};

    Result.WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    Result.WindowClass.lpfnWndProc = MainWindowCallback;
    Result.WindowClass.hInstance = GetModuleHandle(NULL); // Get the handle of exe
    //    WindowClass.hIcon;
    Result.WindowClass.lpszClassName = "CalendarWindowClass";

    if( RegisterClass(&Result.WindowClass) )
    {
        Result.Handle =
            CreateWindowEx(0,
                           Result.WindowClass.lpszClassName,
                           "Calendar",
                           WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                           CW_USEDEFAULT, //x
                           CW_USEDEFAULT, //y
                           CW_USEDEFAULT, //Width
                           CW_USEDEFAULT, //Height
                           0,
                           0,
                           Result.WindowClass.hInstance, // Get instance of exe
                           0);
    }
    else
    {
        // @Logging
    }

    Assert(Result.Handle);

    if(Result.Handle == NULL)
    {
        // @Logging
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

    GameMain(0, NULL, Window);
    
    return 0;
}

#endif
