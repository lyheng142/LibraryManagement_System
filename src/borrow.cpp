#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <thread>
#include <chrono>
#include "borrow.hpp"
#include "pagination.hpp"
#include "splash.hpp"
#include "ui_helper.hpp"
#include <tabulate/table.hpp>

using namespace std;
using namespace tabulate;

// ═══════════════════════════════════════════════════════════════
//  Borrow class — constructor + method implementations
// ═══════════════════════════════════════════════════════════════
Borrow::Borrow()
    : borrowId(0), bookId(0), returned(false) {}

Borrow::Borrow(int borrowId, const string& username, int bookId,
               const string& bookTitle, const string& borrowDate)
    : borrowId(borrowId), username(username), bookId(bookId),
      bookTitle(bookTitle), borrowDate(borrowDate),
      returnDate(""), returned(false) {}

// ── OOP: markReturned encapsulates the return logic ──────────
void Borrow::markReturned(const string& date) {
    returned   = true;
    returnDate = date;
}

// ── Date helper ───────────────────────────────────────────────
static string getDate() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    ostringstream ss; ss << put_time(ltm, "%Y-%m-%d");
    return ss.str();
}

static int findBookIndex(const vector<Book>& books, int id) {
    for (int i = 0; i < (int)books.size(); i++)
        if (books[i].getId() == id) return i;
    return -1;
}

// ═══════════════════════════════════════════════════════════════
//  Render a page of borrow records
// ═══════════════════════════════════════════════════════════════
static void renderBorrowPage(const vector<Borrow>& page, int /*si*/) {
    Table t;
    t.add_row({"#", "User", "Book", "Borrowed", "Returned", "Status"});

    for (const auto& b : page) {
        // ── OOP: use getters ─────────────────────────────────
        t.add_row({
            to_string(b.getBorrowId()),
            b.getUsername(),
            b.getBookTitle(),
            b.getBorrowDate(),
            b.isReturned() ? b.getReturnDate() : "-",
            b.isReturned() ? "Returned" : "Active"
        });
    }

    t[0].format().font_style({FontStyle::bold}).font_color(Color::yellow)
        .border_bottom_color(Color::cyan);
    t.format().border_color(Color::cyan);

    for (size_t i = 1; i < t.size(); ++i) {
        t[i].format().font_color(i % 2 == 1 ? Color::cyan : Color::white);
        // ── OOP: use isReturned() method ──────────────────────
        t[i][5].format()
            .font_color(page[i-1].isReturned() ? Color::green : Color::yellow)
            .font_style({FontStyle::bold});
    }

    for (size_t i = 0; i < t.size(); ++i) {
        t[i][0].format().width(4) .font_align(FontAlign::center);
        t[i][1].format().width(12).font_align(FontAlign::left);
        t[i][2].format().width(20).font_align(FontAlign::left);
        t[i][3].format().width(12).font_align(FontAlign::center);
        t[i][4].format().width(12).font_align(FontAlign::center);
        t[i][5].format().width(9) .font_align(FontAlign::center);
    }

    printCenteredBlankLine();
    printCentered(t);
}

// ═══════════════════════════════════════════════════════════════
//  BORROW BOOK
// ═══════════════════════════════════════════════════════════════
void borrowBook(vector<Book>& books, vector<Borrow>& borrows, const string& username) {
    clearScreen();
    printFigletTitle("BORROW", SP_CYAN);
    printBanner("  Borrow a book from the library  ", Color::cyan);

    if (books.empty()) { printStatus("No books available.", false); return; }

    // Show available books
    Table avail;
    avail.add_row({"ID", "Title", "Author", "Available"});
    for (const auto& b : books)
        if (b.isAvailable())    // ── OOP: use isAvailable() method
            avail.add_row({to_string(b.getId()), b.getTitle(),
                           b.getAuthor(), to_string(b.getQuantity())});
    avail[0].format().font_style({FontStyle::bold}).font_color(Color::yellow)
        .border_bottom_color(Color::cyan);
    avail.format().border_color(Color::cyan);
    for (size_t i = 0; i < avail.size(); ++i) {
        avail[i][0].format().width(5).font_align(FontAlign::center);
        avail[i][1].format().width(22); avail[i][2].format().width(18);
        avail[i][3].format().width(9).font_align(FontAlign::center);
    }
    printCenteredBlankLine(); printCentered(avail);

    printPrompt("Enter Book ID to borrow", Color::cyan);
    int id; cin >> id;

    int index = findBookIndex(books, id);
    if (index == -1) { printStatus("Book not found.", false); return; }
    if (!books[index].isAvailable()) {  // ── OOP: isAvailable()
        printStatus("\"" + books[index].getTitle() + "\" is out of stock!", false); return;
    }

    // Check duplicate borrow
    for (const auto& br : borrows)
        if (br.getBookId() == id && br.belongsTo(username) && br.isActive()) {
            printStatus("You already have this book and haven't returned it.", false); return;
        }

    // ── OOP: use Book method decreaseQty() ────────────────────
    int newId = borrows.empty() ? 1 : borrows.back().getBorrowId() + 1;
    Borrow br(newId, username, books[index].getId(),
              books[index].getTitle(), getDate());
    books[index].decreaseQty();   // OOP method!
    borrows.push_back(br);

    printCard("BORROW RECEIPT", {
        {"Book",      br.getBookTitle()},
        {"User",      br.getUsername()},
        {"Date",      br.getBorrowDate()},
        {"Remaining", to_string(books[index].getQuantity()) + " copies left"}
    }, Color::cyan);

    printStatus("\"" + br.getBookTitle() + "\" borrowed successfully!", true);
    this_thread::sleep_for(chrono::milliseconds(1000));
}

// ═══════════════════════════════════════════════════════════════
//  RETURN BOOK
// ═══════════════════════════════════════════════════════════════
void returnBook(vector<Book>& books, vector<Borrow>& borrows, const string& username) {
    clearScreen();
    printFigletTitle("RETURN", SP_GREEN);
    printBanner("  Return a borrowed book  ", Color::green);

    // ── OOP: use isActive() and belongsTo() ───────────────────
    vector<Borrow*> active;
    for (auto& b : borrows)
        if (b.belongsTo(username) && b.isActive()) active.push_back(&b);

    if (active.empty()) { printStatus("You have no active borrows.", false); return; }

    Table act;
    act.add_row({"Book ID", "Title", "Borrowed On"});
    for (auto* br : active)
        act.add_row({to_string(br->getBookId()), br->getBookTitle(), br->getBorrowDate()});
    act[0].format().font_style({FontStyle::bold}).font_color(Color::yellow)
        .border_bottom_color(Color::green);
    act.format().border_color(Color::green);
    for (size_t i = 0; i < act.size(); ++i) {
        act[i][0].format().width(8).font_align(FontAlign::center);
        act[i][1].format().width(22); act[i][2].format().width(12).font_align(FontAlign::center);
    }
    printCenteredBlankLine(); printCentered(act);

    printPrompt("Enter Book ID to return", Color::green);
    int id; cin >> id;

    for (auto& b : borrows) {
        if (b.getBookId() == id && b.belongsTo(username) && b.isActive()) {
            string today = getDate();
            b.markReturned(today);   // ── OOP: use markReturned() method!

            int index = findBookIndex(books, id);
            if (index != -1) books[index].increaseQty();  // OOP method!

            printCard("RETURN RECEIPT", {
                {"Book",     b.getBookTitle()},
                {"Borrowed", b.getBorrowDate()},
                {"Returned", b.getReturnDate()}
            }, Color::green);

            printStatus("\"" + b.getBookTitle() + "\" returned successfully!", true);
            this_thread::sleep_for(chrono::milliseconds(1000));
            return;
        }
    }
    printStatus("No active borrow record found for Book ID " + to_string(id) + ".", false);
}

// ═══════════════════════════════════════════════════════════════
//  VIEW BORROW HISTORY  (with pagination)
//  username="" → show all (librarian); otherwise only that user's
// ═══════════════════════════════════════════════════════════════
void viewBorrowHistory(const vector<Borrow>& borrows, const string& username) {
    clearScreen();
    bool isAll = username.empty();
    printFigletTitle("HISTORY", isAll ? SP_MAG : SP_CYAN);
    printBanner(isAll ? "  All borrow records  " : "  My borrow history  ",
                isAll ? Color::magenta : Color::cyan);

    // ── OOP: use belongsTo() method to filter ─────────────────
    vector<Borrow> filtered;
    for (const auto& b : borrows)
        if (username.empty() || b.belongsTo(username)) filtered.push_back(b);

    if (filtered.empty()) { printStatus("No records found.", false); return; }

    int active = 0, returned = 0;
    for (const auto& b : filtered)
        b.isReturned() ? returned++ : active++;

    Color col = isAll ? Color::magenta : Color::cyan;
    printCard("SUMMARY", {
        {"Total",    to_string(filtered.size())},
        {"Active",   to_string(active)},
        {"Returned", to_string(returned)}
    }, col);

    paginate<Borrow>(filtered, "Borrow History", col,
        [](const vector<Borrow>& page, int si) { renderBorrowPage(page, si); });
}

// ═══════════════════════════════════════════════════════════════
//  VIEW OVERDUE (not returned)
// ═══════════════════════════════════════════════════════════════
void viewOverdueBooks(const vector<Borrow>& borrows) {
    clearScreen();
    printFigletTitle("OVERDUE", SP_RED);
    printBanner("  Books not yet returned  ", Color::red);

    // ── OOP: use isActive() method ────────────────────────────
    vector<Borrow> overdue;
    for (const auto& b : borrows)
        if (b.isActive()) overdue.push_back(b);

    if (overdue.empty()) { printStatus("No overdue books! Everyone returned.", true); return; }

    printStatus(to_string(overdue.size()) + " book(s) still out.", false);

    paginate<Borrow>(overdue, "Overdue Books", Color::red,
        [](const vector<Borrow>& page, int si) { renderBorrowPage(page, si); });
}
