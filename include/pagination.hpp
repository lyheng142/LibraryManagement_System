#ifndef PAGINATION_HPP
#define PAGINATION_HPP

// ═══════════════════════════════════════════════════════════════
//  Pagination helper — shows any vector page by page
//  PAGE_SIZE books per page, user presses N/P/Q to navigate
// ═══════════════════════════════════════════════════════════════

#include <vector>
#include <string>
#include <functional>
#include <iostream>
#include <tabulate/table.hpp>
#include "terminal_layout.hpp"
#include "splash.hpp"
#include "ui_helper.hpp"

using namespace std;
using namespace tabulate;

static const int PAGE_SIZE = 5;   // books shown per page

// Generic paginator
// renderPage(items_on_this_page, first_index)  → builds and prints the table
template<typename T>
void paginate(const vector<T>& items,
              const string&    title,
              Color            borderColor,
              function<void(const vector<T>&, int)> renderPage)
{
    if (items.empty()) {
        printStatus("No records to display.", false);
        return;
    }

    int totalPages = ((int)items.size() + PAGE_SIZE - 1) / PAGE_SIZE;
    int page       = 0;   // 0-indexed

    while (true) {
        int start = page * PAGE_SIZE;
        int end   = min(start + PAGE_SIZE, (int)items.size());

        vector<T> pageItems(items.begin() + start, items.begin() + end);

        // Call the caller-provided renderer
        renderPage(pageItems, start);

        // ── Page navigation bar ───────────────────────────────────
        Table nav;
        string info = "  Page " + to_string(page + 1) + " / " + to_string(totalPages)
                    + "   |   Records " + to_string(start + 1)
                    + " - " + to_string(end) + " of " + to_string(items.size()) + "  ";
        nav.add_row({info});
        nav[0].format()
            .font_align(FontAlign::center)
            .font_style({FontStyle::bold})
            .font_color(borderColor)
            .border_color(borderColor)
            .padding_top(0).padding_bottom(0)
            .width(TBL_W);
        printCentered(nav);

        // ── Navigation options ────────────────────────────────────
        Table opts;
        string navStr = " ";
        if (page > 0)              navStr += "[P] Prev  ";
        if (page < totalPages - 1) navStr += "[N] Next  ";
        navStr += "[Q] Back ";
        opts.add_row({navStr});
        opts[0].format()
            .font_align(FontAlign::center)
            .font_color(Color::yellow)
            .border_color(Color::yellow)
            .font_style({FontStyle::bold})
            .padding_top(0).padding_bottom(0)
            .width(TBL_W);
        printCentered(opts);

        cout << string(centeredLeftPad(TBL_W), ' ')
             << SP_YELLOW SP_BOLD ">> Navigate: " SP_RESET;

        char ch; cin >> ch; ch = tolower(ch);
        cin.ignore(1000, '\n');

        if (ch == 'n' && page < totalPages - 1) page++;
        else if (ch == 'p' && page > 0)         page--;
        else if (ch == 'q')                      break;
        else {
            printStatus("Press N (next), P (prev), or Q (back).", false);
        }
    }
}

#endif
