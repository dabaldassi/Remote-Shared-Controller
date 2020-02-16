#include <map>

#include <Windows.h>

#include <controller.h>

static HHOOK keyboard_hook = NULL;
static HHOOK mouse_hook = NULL;

LRESULT CALLBACK LowLevelKeyboardProc(
    _In_ int    nCode,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    if (nCode < 0) return CallNextHookEx(NULL, nCode, wParam, lParam);

    PostMessage(NULL, KEY, wParam, lParam);

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}


LRESULT CALLBACK LowLevelMouseProc(
    _In_ int    nCode,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    if (nCode < 0) return CallNextHookEx(NULL, nCode, wParam, lParam);

    PostMessage(NULL, MOUSE, wParam, lParam);

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int init_controller()
{
    if (keyboard_hook != NULL || mouse_hook != NULL) return 1;

    keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    if(keyboard_hook == NULL) return 1;

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
}

int fill_mouse_ev(ControllerEvent* ev, WPARAM mouse_ev, const MSLLHOOKSTRUCT& mouse)
{
    switch (mouse_ev) {
    case WM_LBUTTONDOWN:
        ev->controller_type = KEY;
        ev->ev_type = EV_KEY;
        ev->value = KEY_PRESSED;
        ev->code = BTN_LEFT;
        break;
    case WM_LBUTTONUP:
        ev->controller_type = KEY;
        ev->ev_type = EV_KEY;
        ev->value = KEY_RELEASED;
        ev->code = BTN_LEFT;
        break;
    case WM_RBUTTONDOWN:
        ev->controller_type = KEY;
        ev->ev_type = EV_KEY;
        ev->value = KEY_PRESSED;
        ev->code = BTN_RIGHT;
        break;
    case WM_RBUTTONUP:
        ev->controller_type = KEY;
        ev->ev_type = EV_KEY;
        ev->value = KEY_RELEASED;
        ev->code = BTN_RIGHT;
        break;
    case WM_MOUSEMOVE:
        ev->controller_type = MOUSE;
        ev->ev_type = EV_ABS;
        ev->value = mouse.pt.x;
        ev->code = ABS_X;
        break;
    case WM_MOUSEWHEEL:
        ev->controller_type = MOUSE;
        ev->ev_type = EV_REL;
        ev->value = ((int)mouse.mouseData < 0) ? -1 : 1;
        ev->code = REL_WHEEL;
        break;
        break;
    case WM_MOUSEHWHEEL:
        ev->controller_type = MOUSE;
        ev->ev_type = EV_REL;
        ev->value = ((int)mouse.mouseData < 0) ? -1 : 1;
        ev->code = REL_HWHEEL;
        break;
    }

    return 0;
}

int poll_controller(ControllerEvent* ev, int timeout)
{
    MSG  msg;
    int  quit = 0;
    
    while (!quit) {
        int err = GetMessage(&msg, NULL, 0, 0);

        if (err <= 0) return -1;

        TranslateMessage(&msg);
       
        if (msg.message == KEY) {
            KBDLLHOOKSTRUCT key = *(KBDLLHOOKSTRUCT*)msg.lParam;
            bool pressed = (msg.wParam == WM_KEYDOWN || msg.wParam == WM_SYSKEYDOWN);
            bool released = (WM_KEYUP == msg.wParam || msg.wParam == WM_SYSKEYUP);

            if (!((key.flags & LLKHF_INJECTED) || (key.flags & LLKHF_LOWER_IL_INJECTED))) {
                ev->controller_type = KEY;
                ev->ev_type = EV_KEY;
                ev->value = (pressed) ? KEY_PRESSED : KEY_RELEASED;
                ev->code = key.scanCode;
                quit = 1;
            }
        }
        else if (msg.message == MOUSE) {
            MSLLHOOKSTRUCT mouse = *(MSLLHOOKSTRUCT*)msg.lParam;
            WPARAM         mouse_ev = msg.wParam;

            if (!((mouse.flags & LLMHF_LOWER_IL_INJECTED) || (mouse.flags & LLMHF_INJECTED))) {
                fill_mouse_ev(ev, mouse_ev, mouse);
                quit = 1;
            }
        }

        DispatchMessage(&msg);
    }
    
    return quit;
}

void grab_controller(bool t)
{
    BlockInput(t);
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
            DWORD      flag = (ce->value == KEY_RELEASED) ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
            fill_input(mouse_input, input, flag);
        }
        else if (ce->code == BTN_RIGHT) {
            MOUSEINPUT mouse_input = {};
            DWORD      flag = (ce->value == KEY_RELEASED) ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
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
            mouse_input.mouseData = ce->value;
            flag |= MOUSEEVENTF_WHEEL;
            break;
        case REL_HWHEEL:
            mouse_input.mouseData = ce->value;
            flag |= MOUSEEVENTF_HWHEEL;
            break;
        case ABS_X: // Same value as REL_X
            flag |= MOUSEEVENTF_MOVE;

            if (ce->ev_type == EV_ABS) {
                POINT p;
                GetCursorPos(&p);
                mouse_input.dx = ce->value;
                mouse_input.dy = p.y;
                flag |= MOUSEEVENTF_ABSOLUTE;
            }
            break;
        case ABS_Y: // Same value as REL_Y
            flag |= MOUSEEVENTF_MOVE;

            if (ce->ev_type == EV_ABS) {
                POINT p;
                GetCursorPos(&p);
                mouse_input.dx = p.x;
                mouse_input.dy = ce->value;
                flag |= MOUSEEVENTF_ABSOLUTE;
            }
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
    SendInput(1, &input, sizeof(int));
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

char* get_key_name(unsigned short code)
{
    return nullptr;
}

char* get_key_name_azerty(unsigned short code)
{
    // MapVirtualKey(code, MAPVK_VSC_TO_VK);
    // ToUnicode

    return nullptr;
}