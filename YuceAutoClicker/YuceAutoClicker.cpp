#include <windows.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <string>

using namespace std;

std::atomic<bool> isRunning(false);
int clickRate = 15;
bool isRightClick = false;
UINT selectedKey = 0;

void SimulateMouseClick(bool rightClick) {
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;

    if (rightClick) {
        input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
        SendInput(1, &input, sizeof(INPUT));
        input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
    }
    else {
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &input, sizeof(INPUT));
        input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    }

    SendInput(1, &input, sizeof(INPUT));
}

void StartClicking() {
    while (isRunning) {
        if (GetAsyncKeyState(selectedKey) & 0x8000) {
            SimulateMouseClick(isRightClick);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / clickRate));
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    static HWND cpsInputBox, clickTypeComboBox, keyButton, startButton, infoText;

    switch (msg) {
    case WM_CREATE:
        SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(230, 230, 230)));

        CreateWindow(L"STATIC", L"YÜCE MAKRO", WS_VISIBLE | WS_CHILD | SS_CENTER,
            10, 10, 320, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);

        CreateWindow(L"STATIC", L"CPS (Tıklama Hızı):", WS_VISIBLE | WS_CHILD | SS_LEFT,
            20, 60, 150, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);

        cpsInputBox = CreateWindow(L"EDIT", L"15", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
            180, 60, 120, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);

        CreateWindow(L"STATIC", L"Tıklama Tipi:", WS_VISIBLE | WS_CHILD | SS_LEFT,
            20, 100, 150, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);

        clickTypeComboBox = CreateWindow(L"COMBOBOX", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | WS_BORDER,
            180, 100, 120, 100, hwnd, NULL, GetModuleHandle(NULL), NULL);

        SendMessage(clickTypeComboBox, CB_ADDSTRING, 0, (LPARAM)L"Sol Tık");
        SendMessage(clickTypeComboBox, CB_ADDSTRING, 0, (LPARAM)L"Sağ Tık");
        SendMessage(clickTypeComboBox, CB_SETCURSEL, 0, 0);

        CreateWindow(L"STATIC", L"Makro Tuşunu Ayarla:", WS_VISIBLE | WS_CHILD | SS_LEFT,
            20, 140, 150, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);

        keyButton = CreateWindow(L"BUTTON", L"Bir Tuş Seçiniz", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            180, 140, 120, 30, hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);

        infoText = CreateWindow(L"STATIC", L"", WS_VISIBLE | WS_CHILD | SS_CENTER,
            20, 180, 300, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);

        startButton = CreateWindow(L"BUTTON", L"Başlat", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            115, 220, 100, 30, hwnd, (HMENU)2, GetModuleHandle(NULL), NULL);
        break;

    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case 1: {
            SetWindowText(infoText, L"Lütfen bir tuşa basınız...");
            for (;;) {
                for (UINT key = 0x08; key <= 0xFF; ++key) {
                    if (GetAsyncKeyState(key) & 0x8000) {
                        selectedKey = key;
                        WCHAR msg[100];
                        swprintf_s(msg, 100, L"Makro tuşu seçildi: %u", selectedKey);
                        SetWindowText(infoText, msg);
                        SetWindowText(keyButton, L"Tuş Ayarlandı");
                        return 0;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            break;
        }
        case 2: {
            if (selectedKey == 0) {
                MessageBox(hwnd, L"Lütfen önce bir tuş seçiniz.", L"Hata", MB_OK);
                break;
            }

            WCHAR buffer[256] = { 0 };
            GetWindowText(cpsInputBox, buffer, 256);
            int newRate = _wtoi(buffer);
            if (newRate > 0) {
                clickRate = newRate;
            }
            else {
                MessageBox(hwnd, L"Geçerli bir CPS değeri girin.", L"Hata", MB_OK);
                break;
            }

            int selected = SendMessage(clickTypeComboBox, CB_GETCURSEL, 0, 0);
            isRightClick = (selected == 1);

            isRunning = true;
            std::thread clickThread(StartClicking);
            clickThread.detach();
            ShowWindow(hwnd, SW_MINIMIZE);
            break;
        }
        }
        break;

    case WM_CLOSE:
        isRunning = false;
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"YuceMacro";
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, L"YuceMacro", L"Yüce Makro",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        300, 200, 350, 300, NULL, NULL, hInstance, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}