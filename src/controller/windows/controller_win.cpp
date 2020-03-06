#include <map>

#include <Windows.h>

#include <controller.h>

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC ((unsigned short) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE ((unsigned short) 0x02)
#endif

constexpr size_t RID_LENGTH = 1;

static RAWINPUTDEVICE raw_input_device[RID_LENGTH];
static BOOL  should_grab = FALSE;
static HHOOK keyboard_hook = NULL;
static HHOOK mouse_hook = NULL;
static HWND  window = NULL;

#define KEY_MSG 5
#define MOUSE_MSG 2

struct {
    struct {
        int x = 0;
        int y = 0;
        int wheel = 0;
        int hwheel = 0;
    } mouse;

    struct {
        int code = 0;
        int pressed = 0;
    } kbd;

} input;

LRESULT CALLBACK LowLevelKeyboardProc(
    _In_ int    nCode,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    if (nCode < 0) return CallNextHookEx(NULL, nCode, wParam, lParam);

    KBDLLHOOKSTRUCT* kbd = (KBDLLHOOKSTRUCT*)lParam;
    
    if (!(kbd->flags & LLKHF_INJECTED) && !(kbd->flags & LLKHF_LOWER_IL_INJECTED)) {
        bool pressed = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
        input.kbd.code = kbd->scanCode;
        input.kbd.pressed = (pressed) ? KEY_PRESSED : KEY_RELEASED;
        PostMessage(NULL, KEY_MSG, 0, 0);
    }

    if (should_grab) return TRUE; // Prevent other process to get the event

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}


LRESULT CALLBACK LowLevelMouseProc(
    _In_ int    nCode,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    if (nCode < 0) return CallNextHookEx(NULL, nCode, wParam, lParam);

    MSLLHOOKSTRUCT* mouse = (MSLLHOOKSTRUCT*)lParam;

    if (!(mouse->flags & LLMHF_LOWER_IL_INJECTED) && !(mouse->flags & LLMHF_INJECTED)) {
        UINT msg_type;
        WPARAM mouse_param = 0;

        switch (wParam) {
        case WM_LBUTTONDOWN:
            input.kbd.code = BTN_LEFT;
            input.kbd.pressed = KEY_PRESSED;
            msg_type = KEY_MSG;
            break;
        case WM_LBUTTONUP:
            input.kbd.code = BTN_LEFT;
            input.kbd.pressed = KEY_RELEASED;
            msg_type = KEY_MSG;
            break;
        case WM_RBUTTONDOWN:
            input.kbd.code = BTN_RIGHT;
            input.kbd.pressed = KEY_PRESSED;
            msg_type = KEY_MSG;
            break;
        case WM_RBUTTONUP:
            input.kbd.code = BTN_RIGHT;
            input.kbd.pressed = KEY_RELEASED;
            msg_type = KEY_MSG;
            break;
        case WM_MOUSEWHEEL:
            msg_type = MOUSE_MSG;
            mouse_param = REL_WHEEL;
            break;
        case WM_MOUSEHWHEEL:
            msg_type = MOUSE_MSG;
            mouse_param = REL_HWHEEL;
            break;
        default:
            msg_type = MOUSE_MSG;
            PostMessage(NULL, msg_type, REL_X, 0);
            mouse_param = REL_Y;
            break;
        }

        PostMessage(NULL, msg_type, mouse_param, 0);
    }

    if (should_grab) return TRUE;

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK mouse_event_handler(HWND hwnd,unsigned event,WPARAM wparam,LPARAM lparam)
{
    switch (event) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_INPUT: {
        unsigned size = sizeof(RAWINPUT);
        static RAWINPUT raw[sizeof(RAWINPUT)];
        GetRawInputData((HRAWINPUT)lparam, RID_INPUT, raw, &size, sizeof(RAWINPUTHEADER));

        if (raw->header.dwType == RIM_TYPEMOUSE) {
            input.mouse.x = raw->data.mouse.lLastX;
            input.mouse.y = raw->data.mouse.lLastY;

            if (raw->data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
                input.mouse.wheel = (*(short*)&raw->data.mouse.usButtonData) / WHEEL_DELTA;
        }
    } return 0;
    }

    // Run default message processor for any missed events:
    return DefWindowProc(hwnd, event, wparam, lparam);
}

int init_controller()
{
    if (keyboard_hook != NULL || mouse_hook != NULL) return 1;

    keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    if(keyboard_hook == NULL) return 1;

    /*
        Create a ghost window just to listen de raw input mouse
    */

    HINSTANCE instance = GetModuleHandle(0);

    // Create message-only window:
    const TCHAR class_name[] = TEXT("ghost");

    WNDCLASS window_class = {};
    window_class.lpfnWndProc = mouse_event_handler;
    window_class.hInstance = instance;
    window_class.lpszClassName = class_name;

    if (!RegisterClass(&window_class)) return 2;

    window = CreateWindow(class_name, class_name, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, 0, 0);

    if (window == nullptr) return 1;

    /* Register raw mouse device */

    raw_input_device[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    raw_input_device[0].usUsage = HID_USAGE_GENERIC_MOUSE;
    raw_input_device[0].dwFlags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY;
    raw_input_device[0].hwndTarget = window;

    RegisterRawInputDevices(raw_input_device, RID_LENGTH, sizeof(raw_input_device[0]));

    mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);
    if (mouse_hook == NULL) {
        UnhookWindowsHookEx(keyboard_hook);
        keyboard_hook = NULL;
        return 1;
    }

    return 0;
}

void exit_controller()
{
    if(keyboard_hook) {
        UnhookWindowsHookEx(keyboard_hook);
        keyboard_hook = NULL;
    }
    if(mouse_hook) {
        UnhookWindowsHookEx(mouse_hook);
        mouse_hook = NULL;
    }

    if (window) {
        DestroyWindow(window);
        window = NULL;

        const TCHAR class_name[] = TEXT("ghost");
        HINSTANCE instance = GetModuleHandle(0);
        BOOL r = UnregisterClass(class_name, instance);
    }
}

int poll_controller(ControllerEvent* ev, int timeout)
{
    MSG  msg;
    int  quit = 0;

    while (!quit) {
        int err = GetMessage(&msg, NULL, 0, 0);
        if (err <= 0) return -1;
        TranslateMessage(&msg);
       
        if (msg.message == KEY_MSG) {
            ev->controller_type = KEY;
            ev->ev_type = EV_KEY;
            ev->value = input.kbd.pressed;
            ev->code = input.kbd.code;
            quit = 1;
        }
        else if (msg.message == MOUSE_MSG) {

            ev->controller_type = MOUSE;
            ev->ev_type = EV_REL;
            ev->code = msg.wParam;
            
            switch (ev->code) {
            case REL_X : ev->value = input.mouse.x; break;
            case REL_Y: ev->value = input.mouse.y; break;
            case REL_WHEEL: ev->value = input.mouse.wheel; break;
            case REL_HWHEEL: ev->value = input.mouse.hwheel; break;
            }

            quit = 1;
        }

        DispatchMessage(&msg);
    }
    
    return quit;
}

void grab_controller(bool t)
{
    should_grab = t;
}

void fill_input(MOUSEINPUT& input_type, INPUT& input, DWORD flags)
{
    input_type.dwFlags = flags;
    input.type = INPUT_MOUSE;
    input.mi = input_type;
}

void fill_input(KEYBDINPUT& input_type, INPUT& input, DWORD flags)
{
    input_type.dwFlags = flags;
    input.type = INPUT_KEYBOARD;
    input.ki = input_type;
}

void write_controller(const ControllerEvent* ce)
{
    constexpr int nb_input = 1;

    INPUT input = {};

    if (ce->controller_type == KEY) {
        if (ce->code == BTN_LEFT) {
            MOUSEINPUT mouse_input = {};
            DWORD      flag = (ce->value == KEY_PRESSED) ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
            fill_input(mouse_input, input, flag);
        }
        else if (ce->code == BTN_RIGHT) {
            MOUSEINPUT mouse_input = {};
            DWORD      flag = (ce->value == KEY_PRESSED) ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
            fill_input(mouse_input, input, flag);
        }
        else {
            KEYBDINPUT key_input = {};
            DWORD      flag = KEYEVENTF_SCANCODE; // ignore key_input.wVk
            key_input.wScan = ce->code;

            if (ce->value == KEY_RELEASED) flag |= KEYEVENTF_KEYUP;
            fill_input(key_input, input, flag);
        }
    }
    else if (ce->controller_type == MOUSE) {
        MOUSEINPUT mouse_input = {};
        DWORD      flag = 0x00;
        
        switch (ce->code) {
        case REL_WHEEL:
            if(ce->value > 0)      mouse_input.mouseData = WHEEL_DELTA;
            else if(ce->value < 0) mouse_input.mouseData = -WHEEL_DELTA;
            else                   mouse_input.mouseData = 0;
            flag |= MOUSEEVENTF_WHEEL;
            break;
        case REL_HWHEEL:
            if (ce->value > 0)      mouse_input.mouseData = WHEEL_DELTA;
            else if (ce->value < 0) mouse_input.mouseData = -WHEEL_DELTA;
            else                    mouse_input.mouseData = 0;
            flag |= MOUSEEVENTF_HWHEEL;
            break;
        case REL_X:
            flag |= MOUSEEVENTF_MOVE;
            mouse_input.dx = ce->value;
            mouse_input.dy = 0;
            break;
        case REL_Y:
            flag |= MOUSEEVENTF_MOVE;
            mouse_input.dx = 0;
            mouse_input.dy = ce->value;
            break;
        }

        fill_input(mouse_input, input, flag);
    }

    SendInput(nb_input, &input, sizeof(input));
}

void write_key(unsigned char c)
{
    KEYBDINPUT key_input = {};
    INPUT      input = {};
    DWORD      flag = KEYEVENTF_SCANCODE;
    
    key_input.wScan = c;

    fill_input(key_input, input, flag);
    SendInput(1, &input, sizeof(input));

    flag |= KEYEVENTF_KEYUP;

    fill_input(key_input, input, flag);
    SendInput(1, &input, sizeof(input));
}

void write_key_ev(unsigned char c, int mode)
{
    KEYBDINPUT key_input = {};
    INPUT      input = {};
    DWORD      flag = KEYEVENTF_SCANCODE;

    key_input.wScan = c;

    if (mode == KEY_RELEASED) flag |= KEYEVENTF_KEYUP;

    fill_input(key_input, input, flag);
    SendInput(1, &input, sizeof(input));
}

bool get_key(unsigned short* key_code, int* value)
{
    return false;
}

void mouse_move(int x, int y)
{
    MOUSEINPUT mouse_input = {};
    INPUT      input = {};
    DWORD      flag = MOUSEEVENTF_MOVE;

    mouse_input.dx = x;
    mouse_input.dy = y;
    fill_input(mouse_input, input, flag);

    SendInput(1, &input, sizeof(input));
}