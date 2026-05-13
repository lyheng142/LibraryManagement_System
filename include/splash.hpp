#ifndef SPLASH_HPP
#define SPLASH_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <map>
#include <array>
#include <cctype>
#include <cstdio>
#include "terminal_layout.hpp"

#ifdef _WIN32
  #include <windows.h>
#else
  #include <unistd.h>
#endif

#define SP_RESET  "\033[0m"
#define SP_BOLD   "\033[1m"
#define SP_MAG    "\033[35;1m"
#define SP_CYAN   "\033[36;1m"
#define SP_YELLOW "\033[33;1m"
#define SP_GREEN  "\033[32;1m"
#define SP_RED    "\033[31;1m"
#define SP_WHITE  "\033[37;1m"
#define SP_BLUE   "\033[34;1m"

inline void initConsole() {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD  m = 0;
    if (GetConsoleMode(h, &m))
        SetConsoleMode(h, m | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleOutputCP(65001);
#endif
}

inline void clearScreen() {
    std::cout << "\033[2J\033[H" << std::flush;
}

inline void printCompactTitle(const std::string& text, const char* color) {
    const std::string inner = "  " + text + "  ";
    const std::string edge(inner.size() + 2, '=');
    printCenteredBlankLine();
    printCenteredText(std::string(color) + SP_BOLD + edge + SP_RESET, true);
    printCenteredText(std::string(color) + SP_BOLD + "|" + inner + "|" + SP_RESET, true);
    printCenteredText(std::string(color) + SP_BOLD + edge + SP_RESET, true);
    printCenteredBlankLine();
}

static const std::map<char, std::array<std::string,6>> ANSI_SHADOW = {
    {' ', {"    ","    ","    ","    ","    ","    "}},
    {'A', {" █████╗ ","██╔══██╗","███████║","██╔══██║","██║  ██║","╚═╝  ╚═╝"}},
    {'B', {"██████╗ ","██╔══██╗","██████╔╝","██╔══██╗","██████╔╝","╚═════╝ "}},
    {'C', {" ██████╗","██╔════╝","██║     ","██║     ","╚██████╗"," ╚═════╝"}},
    {'D', {"██████╗ ","██╔══██╗","██║  ██║","██║  ██║","██████╔╝","╚═════╝ "}},
    {'E', {"███████╗","██╔════╝","█████╗  ","██╔══╝  ","███████╗","╚══════╝"}},
    {'F', {"███████╗","██╔════╝","█████╗  ","██╔══╝  ","██║     ","╚═╝     "}},
    {'G', {" ██████╗ ","██╔════╝ ","██║  ███╗","██║   ██║","╚██████╔╝"," ╚═════╝ "}},
    {'H', {"██╗  ██╗","██║  ██║","███████║","██╔══██║","██║  ██║","╚═╝  ╚═╝"}},
    {'I', {"██╗","██║","██║","██║","██║","╚═╝"}},
    {'J', {"     ██╗","     ██║","     ██║","██   ██║","╚█████╔╝"," ╚════╝ "}},
    {'K', {"██╗  ██╗","██║ ██╔╝","█████╔╝ ","██╔═██╗ ","██║  ██╗","╚═╝  ╚═╝"}},
    {'L', {"██╗     ","██║     ","██║     ","██║     ","███████╗","╚══════╝"}},
    {'M', {"███╗   ███╗","████╗ ████║","██╔████╔██║","██║╚██╔╝██║","██║ ╚═╝ ██║","╚═╝     ╚═╝"}},
    {'N', {"███╗   ██╗","████╗  ██║","██╔██╗ ██║","██║╚██╗██║","██║ ╚████║","╚═╝  ╚═══╝"}},
    {'O', {" ██████╗ ","██╔═══██╗","██║   ██║","██║   ██║","╚██████╔╝"," ╚═════╝ "}},
    {'P', {"██████╗ ","██╔══██╗","██████╔╝","██╔═══╝ ","██║     ","╚═╝     "}},
    {'R', {"██████╗ ","██╔══██╗","██████╔╝","██╔══██╗","██║  ██║","╚═╝  ╚═╝"}},
    {'S', {"███████╗","██╔════╝","███████╗","╚════██║","███████║","╚══════╝"}},
    {'T', {"████████╗","╚══██╔══╝","   ██║   ","   ██║   ","   ██║   ","   ╚═╝   "}},
    {'U', {"██╗   ██╗","██║   ██║","██║   ██║","██║   ██║","╚██████╔╝"," ╚═════╝ "}},
    {'V', {"██╗   ██╗","██║   ██║","██║   ██║","╚██╗ ██╔╝"," ╚████╔╝ ","  ╚═══╝  "}},
    {'W', {"██╗    ██╗","██║    ██║","██║ █╗ ██║","██║███╗██║","╚███╔███╔╝"," ╚══╝╚══╝ "}},
    {'X', {"██╗  ██╗","╚██╗██╔╝"," ╚███╔╝ "," ██╔═██╗ ","██╔╝ ██╗","╚═╝  ╚═╝"}},
    {'Y', {"██╗   ██╗","╚██╗ ██╔╝"," ╚████╔╝ ","  ╚██╔╝  ","   ██║   ","   ╚═╝   "}},
    {'Z', {"███████╗","╚════██║","    ██╔╝","   ██╔╝ ","███████║","╚══════╝"}},
};

inline void printAnsiShadow(const std::string& text, const char* color) {
    std::array<std::string,6> rows = {"","","","","",""};
    for (int ci = 0; ci < (int)text.size(); ci++) {
        char u = (char)std::toupper((unsigned char)text[ci]);
        if (ci > 0 && u != ' ') {
            char prevU = (char)std::toupper((unsigned char)text[ci-1]);
            if (prevU != ' ') for (int r = 0; r < 6; r++) rows[r] += " ";
        }
        auto it = ANSI_SHADOW.find(u);
        if (it == ANSI_SHADOW.end()) continue;
        for (int r = 0; r < 6; r++) rows[r] += it->second[r];
    }
    std::ostringstream block;
    for (int r = 0; r < 6; r++) {
        block << rows[r];
        if (r < 5) block << '\n';
    }
    if (maxVisibleLineWidth(block.str()) + 4 > getTerminalWidth()) {
        printCompactTitle(text, color);
        return;
    }
    std::cout << "\n";
    for (int r = 0; r < 6; r++)
        printCenteredText(std::string(color) + SP_BOLD + rows[r] + SP_RESET, true);
    std::cout << "\n";
}

#ifndef FIGLET_FONT
  #define FIGLET_FONT "slant"
#endif

inline bool figletAvailable() {
    static int cached = -1;
    if (cached != -1) return cached == 1;
#ifdef _WIN32
    FILE* f = _popen("figlet --version 2>nul", "r");
#else
    FILE* f = popen("figlet --version 2>/dev/null", "r");
#endif
    if (!f) { cached = 0; return false; }
    char buf[16]; bool got = (fgets(buf, sizeof(buf), f) != nullptr);
#ifdef _WIN32
    _pclose(f);
#else
    pclose(f);
#endif
    cached = got ? 1 : 0;
    return got;
}

inline std::string runFiglet(const std::string& text,
                             const std::string& font = FIGLET_FONT,
                             int width = 100) {
    std::string safe;
    for (char c : text)
        if ((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c==' '||c=='-')
            safe += c;
    if (safe.empty()) return "";
    std::string cmd = "figlet -f " + font + " -w " + std::to_string(width)
                    + " \"" + safe + "\" 2>/dev/null";
#ifdef _WIN32
    FILE* pipe = _popen(cmd.c_str(), "r");
#else
    FILE* pipe = popen(cmd.c_str(), "r");
#endif
    if (!pipe) return "";
    std::string result; char buf[256];
    while (fgets(buf, sizeof(buf), pipe)) result += buf;
#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif
    return result;
}

inline void printFigletTitle(const std::string& text, const char* color,
                             const std::string& font = "") {
    if (figletAvailable()) {
        std::string out = runFiglet(text, font.empty() ? FIGLET_FONT : font);
        if (!out.empty()) {
            if (maxVisibleLineWidth(out) + 4 > getTerminalWidth()) {
                printCompactTitle(text, color); return;
            }
            std::cout << "\n";
            std::istringstream ss(out); std::string line;
            while (std::getline(ss, line))
                printCenteredText(std::string(color) + SP_BOLD + line + SP_RESET, true);
            std::cout << "\n"; return;
        }
    }
    printAnsiShadow(text, color);
}

// ── Animated loading bar ──────────────────────────────────────────────────────
inline void showLoadingBar(const std::string& label,
                           const char* barColor = SP_GREEN,
                           int totalMs = 1400) {
    const int BAR = 36, STEP = 50;
    int delay = totalMs / STEP;
    const char* stepLabels[] = {
        " Initializing...  "," Loading data...  ",
        " Applying theme.. "," Almost ready...  "," Done!            "
    };
    printCenteredBlankLine();
    printCenteredText(std::string(barColor) + SP_BOLD + label + SP_RESET, true);
    for (int i = 0; i <= STEP; i++) {
        int pct = (i*100)/STEP, filled = (i*BAR)/STEP;
        int si  = std::min((i*4)/STEP, 4);
        std::ostringstream barLine;
        barLine << barColor << SP_BOLD << "[";
        for (int j = 0; j < BAR; j++)
            barLine << (j < filled ? "\xE2\x96\x88" : "\xE2\x96\x91");
        barLine << "] " << std::setw(3) << pct << "% " << stepLabels[si] << SP_RESET;
        std::cout << "\r" << std::string(getTerminalWidth(), ' ') << "\r";
        printCenteredText(barLine.str());
        std::cout << std::flush;
#ifdef _WIN32
        Sleep(delay);
#else
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
#endif
    }
    std::cout << "\n\n";
}

#include <tabulate/table.hpp>
using namespace tabulate;

static const int BANNER_CONTENT_W = 56;

inline void showBanner(const std::string& text, Color borderColor,
                       Color textColor, bool padV = true) {
    Table t; t.add_row({text});
    t[0].format().font_align(FontAlign::center).font_style({FontStyle::bold})
        .font_color(textColor).border_color(borderColor)
        .padding_left(2).padding_right(2)
        .padding_top(padV ? 1 : 0).padding_bottom(padV ? 1 : 0)
        .width(BANNER_CONTENT_W);
    printCenteredBlankLine(); printCentered(t);
}

// ── Typewriter title effect ───────────────────────────────────────────────────
// Types out the big ANSI shadow letters character by character
inline void printAnsiShadowTypewriter(const std::string& text, const char* color, int charDelayMs = 80) {
    std::string built;
    for (int ci = 0; ci < (int)text.size(); ci++) {
        built += text[ci];

        // Build the 6 rows for what we have so far
        std::array<std::string,6> rows = {"","","","","",""};
        for (int bi = 0; bi < (int)built.size(); bi++) {
            char u = (char)std::toupper((unsigned char)built[bi]);
            if (bi > 0 && u != ' ') {
                char prevU = (char)std::toupper((unsigned char)built[bi-1]);
                if (prevU != ' ') for (int r = 0; r < 6; r++) rows[r] += " ";
            }
            auto it = ANSI_SHADOW.find(u);
            if (it == ANSI_SHADOW.end()) continue;
            for (int r = 0; r < 6; r++) rows[r] += it->second[r];
        }

        // Move cursor up 6 lines (if not first char) to redraw in place
        if (ci > 0) std::cout << "\033[6A";

        // Print all 6 rows
        for (int r = 0; r < 6; r++) {
            // Clear line then print
            std::cout << "\033[2K";
            printCenteredText(std::string(color) + SP_BOLD + rows[r] + SP_RESET, true);
        }
        std::cout << std::flush;

#ifdef _WIN32
        Sleep(charDelayMs);
#else
        std::this_thread::sleep_for(std::chrono::milliseconds(charDelayMs));
#endif
    }
    std::cout << "\n";
}

// ── Library-specific splash ───────────────────────────────────────────────────
inline void showStartupSplash() {
    clearScreen();

    std::cout << "\n";

    // Type out LIBRARY letter by letter
    printAnsiShadowTypewriter("LIBRARY", SP_CYAN, 80);

    // Small pause between words
#ifdef _WIN32
    Sleep(150);
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
#endif

    // Type out SYSTEM letter by letter
    printAnsiShadowTypewriter("SYSTEM", SP_CYAN, 80);

    // Subtitle box fades in after typing
    Table info;
    info.add_row({"  Library Management System  v2.0  "});
    info.add_row({"  Manage books, borrow, return and track history  "});
    info[0].format().font_align(FontAlign::center).font_style({FontStyle::bold})
        .font_color(Color::cyan).border_color(Color::cyan)
        .padding_top(1).padding_bottom(0).width(52);
    info[1].format().font_align(FontAlign::center).font_color(Color::white)
        .border_color(Color::cyan).padding_top(0).padding_bottom(1).width(52);
    printCenteredBlankLine(); printCentered(info);

#ifdef _WIN32
    Sleep(300);
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
#endif
    showLoadingBar("Loading Library System...", SP_CYAN, 2000);
}

inline void showDashboardLoading(const std::string& roleWord,
                                  const char* titleColor,
                                  const char* barColor) {
    clearScreen();
    printFigletTitle(roleWord,    titleColor);
    printFigletTitle("DASHBOARD", titleColor);
    showLoadingBar("Loading " + roleWord + " Dashboard", barColor, 1200);
    clearScreen();
}

inline void showLibrarianLoading() { showDashboardLoading("LIBRARIAN", SP_MAG,    SP_MAG);    }
inline void showStudentLoading()   { showDashboardLoading("STUDENT",   SP_CYAN,   SP_CYAN);   }

#endif // SPLASH_HPP