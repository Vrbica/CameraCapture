#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <conio.h>      // _kbhit(), _getch()  — Windows only
#include <windows.h>

#include "canoncontroller.h"

// ---------------------------------------------------------------------------
// Console color helpers
// ---------------------------------------------------------------------------
static HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);

static void colorGreen()  { SetConsoleTextAttribute(hCon, FOREGROUND_GREEN  | FOREGROUND_INTENSITY); }
static void colorRed()    { SetConsoleTextAttribute(hCon, FOREGROUND_RED    | FOREGROUND_INTENSITY); }
static void colorCyan()   { SetConsoleTextAttribute(hCon, FOREGROUND_BLUE   | FOREGROUND_GREEN | FOREGROUND_INTENSITY); }
static void colorYellow() { SetConsoleTextAttribute(hCon, FOREGROUND_RED    | FOREGROUND_GREEN | FOREGROUND_INTENSITY); }
static void colorReset()  { SetConsoleTextAttribute(hCon, FOREGROUND_RED    | FOREGROUND_GREEN | FOREGROUND_BLUE); }

// ---------------------------------------------------------------------------
static void printBanner() {
    colorGreen();
    std::cout << "\n";
    std::cout << "  ==========================================\n";
    std::cout << "         Canon Capture  (pure C++)\n";
    std::cout << "  ==========================================\n";
    colorReset();
}

static void printPrompt(bool capturing) {
    std::cout << "\n";
    colorYellow();
    std::cout << "  Commands:\n";
    if (!capturing) {
        std::cout << "    [S]  Start capture\n";
    } else {
        colorRed();
        std::cout << "    [X]  Stop capture\n";
        colorYellow();
    }
    std::cout << "    [C]  Connect / reconnect camera\n";
    std::cout << "    [Q]  Quit\n";
    colorReset();
    std::cout << "\n> ";
    std::cout.flush();
}

// ---------------------------------------------------------------------------
int main() {
    printBanner();

    CanonController ctrl;

    // --- Wire up callbacks ---
    ctrl.onStatus = [](const std::string& msg) {
        std::cout << "\n";
        colorGreen();
        std::cout << "  [OK]  " << msg;
        colorReset();
        std::cout << "\n> ";
        std::cout.flush();
    };

    ctrl.onError = [](const std::string& msg) {
        std::cout << "\n";
        colorRed();
        std::cout << "  [!!]  " << msg;
        colorReset();
        std::cout << "\n> ";
        std::cout.flush();
    };

    ctrl.onImageSaved = [](const std::string& path) {
        std::cout << "\n";
        colorCyan();
        std::cout << "  [IMG] " << path;
        colorReset();
        std::cout << "\n> ";
        std::cout.flush();
    };

    // --- Initialize SDK ---
    if (!ctrl.initialize()) {
        std::cerr << "Failed to initialize Canon SDK. Exiting.\n";
        return 1;
    }

    // --- Save path ---
    std::cout << "\n  Save path [C:\\Photos]: ";
    std::string savePath;
    std::getline(std::cin, savePath);
    if (savePath.empty()) savePath = "C:\\Photos";
    ctrl.savePath = savePath;

    // --- Part number ---
    std::cout << "  Part number [PART]: ";
    std::string partNumber;
    std::getline(std::cin, partNumber);
    if (partNumber.empty()) partNumber = "PART";
    ctrl.partNumber = partNumber;

    // --- Try to connect camera at startup ---
    std::cout << "\n  Looking for camera...\n";
    ctrl.connectCamera();

    printPrompt(false);

    // --- Main loop ---
    while (true) {
        ctrl.pumpEvents();

        if (_kbhit()) {
            int ch = toupper(_getch());

            if (ch == 'Q') {
                if (ctrl.isCapturing()) ctrl.stopCapture();
                break;
            }
            else if (ch == 'S') {
                if (!ctrl.isConnected()) {
                    ctrl.onError("No camera connected. Press [C] to connect first.");
                } else if (!ctrl.isCapturing()) {
                    ctrl.startCapture();
                    printPrompt(true);
                }
            }
            else if (ch == 'X') {
                if (ctrl.isCapturing()) {
                    ctrl.stopCapture();
                    printPrompt(false);
                }
            }
            else if (ch == 'C') {
                ctrl.disconnectCamera();
                ctrl.connectCamera();
                printPrompt(ctrl.isCapturing());
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    colorGreen();
    std::cout << "\n  Goodbye.\n\n";
    colorReset();
    return 0;
}
