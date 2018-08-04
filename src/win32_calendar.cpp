#if defined(_WIN32) || defined(_WIN64)

#define UNICODE

#include <windows.h>

#include "calendar.cpp"

struct platform_window
{
    HWND Handle;
    HDC DeviceContext;
    WNDCLASS WindowClass;
    char Title[1024];
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
PlatformClearArea(platform_window* Window, u32 OffsetX, u32 OffsetY, u32 Width, u32 Height)
{
}

void
PlatformSetWindowTitle(platform_window* Window, char* Title)
{
}

int
PlatformWindowWidth(platform_window* Window)
{
    return 0;
}

int
PlatformWindowHeight(platform_window* Window)
{
    return 0;
}

void
PlatformCloseWindow(platform_window* Window)
{
    if(Window->Handle)
    {
        CloseWindow(Window->Handle);
    }
}

void
PlatformDrawLine(platform_window* Window, u32 X1, u32 Y1, u32 X2, u32 Y2)
{
}

void
PlatformDrawString(platform_window* Window, u32 PosX, u32 PosY, char* String, u32 StringLength)
{
}
void
PlatformDrawCircle(platform_window* Window, int CenterX, int CenterY, int Radius)
{
}
void
PlatformDrawRectangle(platform_window* Window, u32 TopLeftX, u32 TopLeftY, u32 Width, u32 Height)
{
}

platform_event*
PlatformAllocateMemoryForEvent()
{
    platform_event* Result;
    Result = (platform_event*) malloc(sizeof platform_event);
    return Result;
}

#define WIN32_MESSAGE_RESULT_ERROR -1
b32
PlatformGetNextEvent(platform_window* Window, platform_event* Event)
{
    BOOL MessageResult = GetMessage(&Event->Message, 0, 0, 0);

    Assert(MessageResult != WIN32_MESSAGE_RESULT_ERROR);

    if(MessageResult > 0)
    {
        TranslateMessage(&Event->Message);
        DispatchMessage(&Event->Message);
    }

    return true;
}

timestamp
PlatformGetTime()
{
    timestamp Result = {};

    ULONGLONG SystemTimeInMillisconds = GetTickCount64();

    Result.Seconds      = SystemTimeInMillisconds / 1000;
    Result.Microseconds = (SystemTimeInMillisconds % 1000) * 1000;

    return Result;
}

date_time
PlatformGetLocalDateTime()
{
    date_time Result = {};

    LPSYSTEMTIME SystemTime = {};

    GetLocalTime(SystemTime);

    Result.Year        = SystemTime->wYear;
    Result.Month       = SystemTime->wMonth;
    Result.Day         = SystemTime->wDay;
    Result.Hour        = SystemTime->wHour;
    Result.Minute      = SystemTime->wMinute;
    Result.Second      = SystemTime->wSecond;
    Result.Millisecond = SystemTime->wMilliseconds;

    return Result;
}

b32
PlatformHandleEvent(platform_window Window, platform_event Event)
{
    // NOTE: If this is not being called from the Windows callback, ignore it and early-out.
    if(!Event.fromCallback)
    {
        return true;
    }

    switch(Event.Message.message)
    {
        case WM_SIZE:
        {
            OutputDebugStringA("WM_SIZE\n");
        }
        break;

        case WM_DESTROY:
        {
            OutputDebugStringA("WM_DESTROY\n");
            GlobalIsRunning = false;
        }
        break;

        case WM_CLOSE:
        {
            OutputDebugStringA("WM_CLOSE\n");
            GlobalIsRunning = false;
        }
        break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVEAPP\n");
        }
        break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window.Handle, &Paint);
            int X             = Paint.rcPaint.left;
            int Y             = Paint.rcPaint.top;
            LONG Width        = Paint.rcPaint.right - Paint.rcPaint.left;
            LONG Height       = Paint.rcPaint.bottom - Paint.rcPaint.top;
            PatBlt(DeviceContext, X, Y, Width, Height, WHITENESS);
            EndPaint(Window.Handle, &Paint);
        }
        break;

        default:
        {
            return false;
        }
        break;
    }

    return true;
}

LRESULT CALLBACK
MainWindowCallback(HWND WindowHandle, UINT MessageId, WPARAM WParam, LPARAM LParam)
{
    LRESULT MessageResult = 0;

    platform_window Window = {};
    Window.Handle          = WindowHandle;

    platform_event Event = {};
    Event.fromCallback   = true;
    Event.MessageId      = MessageId;
    Event.WParam         = WParam;
    Event.LParam         = LParam;

    if(!PlatformHandleEvent(Window, Event))
    {
        MessageResult = DefWindowProc(Window.Handle, MessageId, WParam, LParam);
    }

    return MessageResult;
}

platform_window*
PlatformOpenWindow(char* Title, u32 Width, u32 Height)
{
    platform_window* Result = (platform_window*) malloc(sizeof(platform_window));

    Result->WindowClass.style       = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    Result->WindowClass.lpfnWndProc = MainWindowCallback;
    Result->WindowClass.hInstance   = GetModuleHandle(NULL); // Get the handle of exe
    //    WindowClass.hIcon;
    Result->WindowClass.lpszClassName = (LPCWSTR)"CalendarWindowClass";

    if(RegisterClass(&Result->WindowClass))
    {
        Result->Handle
            = CreateWindowEx(0, Result->WindowClass.lpszClassName, (LPCWSTR)"Calendar", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                             CW_USEDEFAULT, // x
                             CW_USEDEFAULT, // y
                             CW_USEDEFAULT, // Width
                             CW_USEDEFAULT, // Height
                             0, 0,
                             Result->WindowClass.hInstance, // Get instance of exe
                             0);
    }
    else
    {
        // @Logging
    }

    Assert(Result->Handle);

    if(Result->Handle == NULL)
    {
        // @Logging
    }

    return Result;
}

int CALLBACK
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode)
{

    platform_window* Window = PlatformOpenWindow("Calendar", 400, 400);

    GameMain(0, NULL, Window);

    return 0;
}

int
main()
{
    GameMain(0, NULL, NULL);
    return 0;
}

#endif
