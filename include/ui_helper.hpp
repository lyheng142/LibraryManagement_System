#ifndef UI_HELPER_HPP
#define UI_HELPER_HPP

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <limits>
#include <ctime>
#include <tabulate/table.hpp>
#include "terminal_layout.hpp"

using namespace tabulate;

static const int TBL_W = 56;

inline std::string fmtPrice(double price) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2) << price;
    return "$" + ss.str();
}


inline void printClockBar(const std::string& role = "", int bookCount = -1) {
    // Get current time
    time_t now = time(0);
    tm* ltm = localtime(&now);

    const char* dayNames[]   = {"SUN","MON","TUE","WED","THU","FRI","SAT"};
    const char* monthNames[] = {"JAN","FEB","MAR","APR","MAY","JUN",
                                "JUL","AUG","SEP","OCT","NOV","DEC"};

    std::ostringstream timeStr, dateStr;
    timeStr << std::setw(2) << std::setfill('0') << ltm->tm_hour << ":"
            << std::setw(2) << std::setfill('0') << ltm->tm_min  << ":"
            << std::setw(2) << std::setfill('0') << ltm->tm_sec;
    dateStr << dayNames[ltm->tm_wday] << " "
            << std::setw(2) << std::setfill('0') << ltm->tm_mday << " "
            << monthNames[ltm->tm_mon] << " "
            << (1900 + ltm->tm_year);

    // Build the bar content
    std::string left  = "  \xF0\x9F\x93\x9A  LIBRARY SYSTEM v2.0";
    std::string mid   = role.empty() ? "" : "  SESSION: " + role;
    std::string right = "  \xE2\x8F\xB0 " + timeStr.str() + "   " + dateStr.str() + "  ";
    if (bookCount >= 0)
        mid += "   BOOKS: " + std::to_string(bookCount);

    // Print as a styled tabulate table spanning full width
    Table bar;
    bar.add_row({left + mid, right});
    bar[0][0].format()
        .font_color(Color::magenta).font_style({FontStyle::bold})
        .font_align(FontAlign::left).width(TBL_W - 28);
    bar[0][1].format()
        .font_color(Color::cyan).font_style({FontStyle::bold})
        .font_align(FontAlign::right).width(28);
    bar[0].format()
        .border_color(Color::magenta)
        .padding_top(0).padding_bottom(0);
    bar.format().border_color(Color::magenta);

    printCenteredBlankLine();
    printCentered(bar);
}

// ─── 1-col section banner ─────────────────────────────────────────────────────
inline void printBanner(const std::string& text, Color color, bool padV = true) {
    Table t;
    t.add_row({text});
    t[0].format()
        .font_align(FontAlign::center).font_style({FontStyle::bold})
        .font_color(color).border_color(color)
        .padding_top(padV ? 1 : 0).padding_bottom(padV ? 1 : 0)
        .width(TBL_W);
    printCenteredBlankLine();
    printCentered(t);
}

inline void printHeader(const std::string& t, Color bc, Color tc) {
    printBanner(t, tc);
    (void)bc;
}

// ─── Input prompt ─────────────────────────────────────────────────────────────
inline void printPrompt(const std::string& label, Color color) {
    std::string col;
    switch(color) {
        case Color::magenta: col = "\033[35;1m"; break;
        case Color::cyan:    col = "\033[36;1m"; break;
        case Color::yellow:  col = "\033[33;1m"; break;
        case Color::red:     col = "\033[31;1m"; break;
        case Color::green:   col = "\033[32;1m"; break;
        default:             col = "\033[37;1m"; break;
    }
    std::cout << std::string(centeredLeftPad(TBL_W), ' ')
              << col << ">> " << label << ": \033[0m";
}

inline void printInputField(const std::string& label, Color color) { printPrompt(label, color); }
inline void printInputFieldClose(Color) { std::cout << "\n"; }

// ─── Status box ───────────────────────────────────────────────────────────────
inline void printStatus(const std::string& msg, bool success = true) {
    Color c = success ? Color::green : Color::red;
    Table t;
    t.add_row({(success ? "  [OK]  " : "  [!!]  ") + msg});
    t[0].format()
        .font_color(c).border_color(c).font_style({FontStyle::bold})
        .font_align(FontAlign::center).padding_top(1).padding_bottom(1)
        .width(TBL_W);
    printCenteredBlankLine();
    printCentered(t);
}

inline void printFormHeader(const std::string& title, const std::string& subtitle, Color color) {
    printBanner(title + "   --   " + subtitle, color, false);
}

inline void printBreadcrumb(const std::vector<std::string>& path, Color color) {
    std::string crumb;
    for (size_t i = 0; i < path.size(); ++i) {
        crumb += "[" + path[i] + "]";
        if (i < path.size() - 1)
            crumb += "  \xE2\x86\x92  ";
    }
    Table t;
    t.add_row({crumb});
    t[0].format()
        .font_align(FontAlign::center).font_color(color).border_color(color)
        .padding_top(0).padding_bottom(0).width(TBL_W);
    printCenteredBlankLine();
    printCentered(t);
}

inline bool confirmAction(const std::string& message, Color color) {
    Table t;
    t.add_row({"  [?]  " + message + "   (y / n)  "});
    t[0].format()
        .font_align(FontAlign::center).font_style({FontStyle::bold})
        .font_color(color).border_color(color)
        .padding_top(1).padding_bottom(1).width(TBL_W);
    printCenteredBlankLine();
    printCentered(t);

    std::string col;
    switch(color) {
        case Color::red:     col = "\033[31;1m"; break;
        case Color::yellow:  col = "\033[33;1m"; break;
        case Color::magenta: col = "\033[35;1m"; break;
        default:             col = "\033[37;1m"; break;
    }
    std::cout << std::string(centeredLeftPad(TBL_W), ' ')
              << col << ">> Confirm (y/n): \033[0m";

    char ch;
    std::cin >> ch;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return (ch == 'y' || ch == 'Y');
}

inline std::string stockBadge(int qty) {
    if (qty == 0)  return "[OUT]";
    if (qty <= 10) return "[LOW]";
    return "[OK] ";
}

inline Color stockColor(int qty) {
    if (qty == 0)  return Color::red;
    if (qty <= 10) return Color::yellow;
    return Color::green;
}

inline void styleProductTable(Table& t, Color borderColor = Color::cyan) {
    t.format().border_color(borderColor).font_color(Color::white);
    t[0].format().font_style({FontStyle::bold}).font_color(Color::yellow)
        .border_bottom_color(borderColor);
    for (size_t i = 1; i < t.size(); ++i)
        t[i].format().font_color(i % 2 == 1 ? Color::cyan : Color::white);
    for (size_t i = 0; i < t.size(); ++i) {
        t[i][0].format().width(5) .font_align(FontAlign::center);
        t[i][1].format().width(20).font_align(FontAlign::left);
        t[i][2].format().width(14).font_align(FontAlign::left);
        t[i][3].format().width(9) .font_align(FontAlign::right);
        t[i][4].format().width(12).font_align(FontAlign::center);
    }
}

inline void applyStockColors(Table& t, const std::vector<int>& quantities) {
    for (size_t i = 0; i < quantities.size(); ++i) {
        t[i + 1][4].format()
            .font_color(stockColor(quantities[i]))
            .font_style({FontStyle::bold});
    }
}

inline void styleCartTable(Table& t) {
    t.format().border_color(Color::cyan).font_color(Color::white);
    t[0].format().font_style({FontStyle::bold}).font_color(Color::yellow)
        .border_bottom_color(Color::cyan);
    for (size_t i = 1; i < t.size(); ++i)
        t[i].format().font_color(i % 2 == 1 ? Color::cyan : Color::white);
    for (size_t i = 0; i < t.size(); ++i) {
        t[i][0].format().width(5) .font_align(FontAlign::center);
        t[i][1].format().width(20).font_align(FontAlign::left);
        t[i][2].format().width(9) .font_align(FontAlign::right);
        t[i][3].format().width(5) .font_align(FontAlign::center);
        t[i][4].format().width(10).font_align(FontAlign::right);
    }
}

inline void printTotalsBox(double subtotal, double tax, double total,
                           Color borderColor = Color::green) {
    Table t;
    t.add_row({"Subtotal",  fmtPrice(subtotal)});
    t.add_row({"Tax (10%)", fmtPrice(tax)});
    t.add_row({"TOTAL",     fmtPrice(total)});
    t.format().border_color(borderColor).font_color(Color::white);
    t[0].format().font_color(Color::white);
    t[1].format().font_color(Color::yellow);
    t[2].format().font_style({FontStyle::bold}).font_color(Color::green)
        .border_top_color(borderColor);
    for (size_t i = 0; i < t.size(); ++i) {
        t[i][0].format().font_style({FontStyle::bold}).font_align(FontAlign::left);
        t[i][1].format().font_align(FontAlign::right);
    }
    printCentered(t);
}

inline void printCard(const std::string& heading,
                      const std::vector<std::pair<std::string,std::string>>& rows,
                      Color color) {
    Table t;
    t.add_row({heading, ""});
    t[0][0].format().width(14).font_align(FontAlign::center)
        .font_style({FontStyle::bold}).font_color(color);
    t[0][1].format().width(38).font_color(Color::white);
    t[0].format().border_color(color).padding_top(1).padding_bottom(1);
    for (size_t i = 0; i < rows.size(); ++i) {
        t.add_row({rows[i].first, rows[i].second});
        t[i+1][0].format().width(14).font_align(FontAlign::right).padding_right(2)
            .font_color(Color::yellow).font_style({FontStyle::bold});
        t[i+1][1].format().width(38).font_color(Color::white);
        t[i+1].format().border_color(color);
    }
    printCenteredBlankLine();
    printCentered(t);
}

#endif // UI_HELPER_HPP