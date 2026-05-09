#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <thread>
#include <chrono>
#include "book.hpp"
#include "pagination.hpp"
#include "splash.hpp"
#include "ui_helper.hpp"
#include <tabulate/table.hpp>

using namespace std;
using namespace tabulate;

// ═══════════════════════════════════════════════════════════════
//  Book class — constructor implementations
// ═══════════════════════════════════════════════════════════════
Book::Book() : id(0), quantity(0) {}

Book::Book(int id, const string& title, const string& author,
           const string& category, int quantity)
    : id(id), title(title), author(author),
      category(category), quantity(quantity) {}

string Book::getStatusLabel() const {
    if (quantity == 0)       return "[OUT]";
    if (quantity <= 3)       return "[LOW]";
    return                          "[OK] ";
}

// ── Check duplicate ID ────────────────────────────────────────
static bool idExists(const vector<Book>& books, int id) {
    for (const auto& b : books)
        if (b.getId() == id) return true;
    return false;
}

// ═══════════════════════════════════════════════════════════════
//  Render a page of books into a tabulate Table
// ═══════════════════════════════════════════════════════════════
static void renderBookPage(const vector<Book>& page, int /*startIndex*/) {
    Table t;
    t.add_row({"ID", "Title", "Author", "Category", "Qty", "Status"});

    for (const auto& b : page) {
        t.add_row({ to_string(b.getId()), b.getTitle(), b.getAuthor(),
                    b.getCategory(), to_string(b.getQuantity()), b.getStatusLabel() });
    }

    // Header styling
    t[0].format().font_style({FontStyle::bold}).font_color(Color::yellow)
        .border_bottom_color(Color::cyan);
    t.format().border_color(Color::cyan);

    // Alternating row colors + status color
    for (size_t i = 1; i < t.size(); ++i) {
        t[i].format().font_color(i % 2 == 1 ? Color::cyan : Color::white);
        int qty = page[i-1].getQuantity();
        Color sc = page[i-1].isOutOfStock() ? Color::red
                 : page[i-1].isLowStock()   ? Color::yellow
                                            : Color::green;
        t[i][5].format().font_color(sc).font_style({FontStyle::bold});
    }

    for (size_t i = 0; i < t.size(); ++i) {
        t[i][0].format().width(5) .font_align(FontAlign::center);
        t[i][1].format().width(22).font_align(FontAlign::left);
        t[i][2].format().width(18).font_align(FontAlign::left);
        t[i][3].format().width(14).font_align(FontAlign::left);
        t[i][4].format().width(5) .font_align(FontAlign::center);
        t[i][5].format().width(7) .font_align(FontAlign::center);
    }

    printCenteredBlankLine();
    printCentered(t);
}

// ═══════════════════════════════════════════════════════════════
//  ADD BOOK
// ═══════════════════════════════════════════════════════════════
void addBook(vector<Book>& books) {
    clearScreen();
    printFigletTitle("ADD BOOK", SP_MAG);
    printBanner("  Enter new book details  ", Color::magenta);

    int    id, qty;
    string title, author, category;

    // Validate unique ID
    do {
        printPrompt("Book ID (number)", Color::magenta);
        if (!(cin >> id) || id <= 0) {
            cin.clear(); cin.ignore(1000,'\n');
            printStatus("ID must be a positive number.", false); id = -1; continue;
        }
        if (idExists(books, id)) {
            printStatus("ID " + to_string(id) + " already exists! Use a different one.", false);
            id = -1;
        }
    } while (id <= 0);

    cin.ignore();
    printPrompt("Title",    Color::magenta); getline(cin, title);
    printPrompt("Author",   Color::magenta); getline(cin, author);
    printPrompt("Category", Color::magenta); getline(cin, category);

    do {
        printPrompt("Quantity", Color::magenta);
        if (!(cin >> qty) || qty < 0) {
            cin.clear(); cin.ignore(1000,'\n');
            printStatus("Quantity must be 0 or more.", false); qty = -1;
        }
    } while (qty < 0);

    // ── OOP: use constructor ───────────────────────────────────
    books.emplace_back(id, title, author, category, qty);

    printCard("BOOK ADDED", {
        {"ID",       to_string(id)},
        {"Title",    title},
        {"Author",   author},
        {"Category", category},
        {"Quantity", to_string(qty)}
    }, Color::magenta);

    printStatus("Book \"" + title + "\" added successfully!", true);
    this_thread::sleep_for(chrono::milliseconds(800));
}

// ═══════════════════════════════════════════════════════════════
//  VIEW BOOKS  (with pagination)
// ═══════════════════════════════════════════════════════════════
void viewBooks(const vector<Book>& books) {
    clearScreen();
    printFigletTitle("BOOKS", SP_CYAN);
    printBanner("  All books in the library  ", Color::cyan);

    if (books.empty()) { printStatus("No books available yet.", false); return; }

    // Summary card
    int totalCopies = 0, outStock = 0, lowStock = 0;
    for (const auto& b : books) {
        totalCopies += b.getQuantity();
        if (b.isOutOfStock()) outStock++;
        else if (b.isLowStock()) lowStock++;
    }
    printCard("LIBRARY SUMMARY", {
        {"Total Titles",  to_string(books.size())},
        {"Total Copies",  to_string(totalCopies)},
        {"Low Stock",     to_string(lowStock)},
        {"Out of Stock",  to_string(outStock)}
    }, Color::cyan);

    // ── OOP: call member method getStatusLabel(), isOutOfStock(), etc.
    paginate<Book>(books, "Book List", Color::cyan,
        [](const vector<Book>& page, int si) {
            renderBookPage(page, si);
        });
}

// ═══════════════════════════════════════════════════════════════
//  SEARCH / FILTER  (with pagination on results)
// ═══════════════════════════════════════════════════════════════
void searchBook(const vector<Book>& books) {
    clearScreen();
    printFigletTitle("SEARCH", SP_YELLOW);
    printBanner("  Search/Filter by title, author, category, or ID  ", Color::yellow);

    printPrompt("Keyword", Color::yellow);
    cin.ignore(); string keyword; getline(cin, keyword);

    string kl = keyword;
    transform(kl.begin(), kl.end(), kl.begin(), ::tolower);

    vector<Book> results;
    for (const auto& b : books) {
        // ── OOP: use getters ─────────────────────────────────
        string tl = b.getTitle(),   al = b.getAuthor(), cl = b.getCategory();
        transform(tl.begin(), tl.end(), tl.begin(), ::tolower);
        transform(al.begin(), al.end(), al.begin(), ::tolower);
        transform(cl.begin(), cl.end(), cl.begin(), ::tolower);
        if (tl.find(kl) != string::npos || al.find(kl) != string::npos ||
            cl.find(kl) != string::npos || to_string(b.getId()) == keyword)
            results.push_back(b);
    }

    if (results.empty()) {
        printStatus("No books found matching \"" + keyword + "\".", false);
        return;
    }
    printStatus("Found " + to_string(results.size()) + " result(s) for \"" + keyword + "\"", true);

    paginate<Book>(results, "Search Results", Color::yellow,
        [](const vector<Book>& page, int si) { renderBookPage(page, si); });
}

// ═══════════════════════════════════════════════════════════════
//  SORT BOOKS  ← NEW FEATURE
// ═══════════════════════════════════════════════════════════════
void sortBooks(vector<Book>& books) {
    clearScreen();
    printFigletTitle("SORT", SP_MAG);
    printBanner("  Sort books by different fields  ", Color::magenta);

    if (books.empty()) { printStatus("No books to sort.", false); return; }

    Table menu;
    menu.add_row({"No", "Sort by",          "Order"});
    menu.add_row({"1",  "ID",               "Ascending"});
    menu.add_row({"2",  "Title",            "A → Z"});
    menu.add_row({"3",  "Author",           "A → Z"});
    menu.add_row({"4",  "Category",         "A → Z"});
    menu.add_row({"5",  "Quantity",         "Low → High"});
    menu.add_row({"6",  "Quantity",         "High → Low"});
    menu.format().border_color(Color::magenta);
    menu[0].format().font_style({FontStyle::bold}).font_color(Color::yellow)
        .border_bottom_color(Color::magenta);
    for (size_t i = 1; i < menu.size(); ++i)
        menu[i].format().font_color(i % 2 == 1 ? Color::cyan : Color::white);
    for (size_t i = 0; i < menu.size(); ++i) {
        menu[i][0].format().width(5) .font_align(FontAlign::center);
        menu[i][1].format().width(16).font_align(FontAlign::left);
        menu[i][2].format().width(14).font_align(FontAlign::left);
    }
    printCenteredBlankLine(); printCentered(menu);

    printPrompt("Choose sort option (1-6)", Color::magenta);
    int choice; cin >> choice;
    if (cin.fail()) { cin.clear(); cin.ignore(1000,'\n');
        printStatus("Invalid input.", false); return; }

    // ── OOP: comparator uses getters ──────────────────────────
    string sortLabel;
    switch (choice) {
        case 1:
            sort(books.begin(), books.end(),
                [](const Book& a, const Book& b){ return a.getId() < b.getId(); });
            sortLabel = "ID (Ascending)"; break;
        case 2:
            sort(books.begin(), books.end(),
                [](const Book& a, const Book& b){ return a.getTitle() < b.getTitle(); });
            sortLabel = "Title (A → Z)"; break;
        case 3:
            sort(books.begin(), books.end(),
                [](const Book& a, const Book& b){ return a.getAuthor() < b.getAuthor(); });
            sortLabel = "Author (A → Z)"; break;
        case 4:
            sort(books.begin(), books.end(),
                [](const Book& a, const Book& b){ return a.getCategory() < b.getCategory(); });
            sortLabel = "Category (A → Z)"; break;
        case 5:
            sort(books.begin(), books.end(),
                [](const Book& a, const Book& b){ return a.getQuantity() < b.getQuantity(); });
            sortLabel = "Quantity (Low → High)"; break;
        case 6:
            sort(books.begin(), books.end(),
                [](const Book& a, const Book& b){ return a.getQuantity() > b.getQuantity(); });
            sortLabel = "Quantity (High → Low)"; break;
        default:
            printStatus("Invalid choice.", false); return;
    }

    printStatus("Books sorted by " + sortLabel + "!", true);
    this_thread::sleep_for(chrono::milliseconds(600));
    viewBooks(books);
}

// ═══════════════════════════════════════════════════════════════
//  UPDATE BOOK
// ═══════════════════════════════════════════════════════════════
void updateBook(vector<Book>& books) {
    clearScreen();
    printFigletTitle("UPDATE", SP_MAG);
    printBanner("  Update book information  ", Color::magenta);

    if (books.empty()) { printStatus("No books to update.", false); return; }
    viewBooks(books);

    printPrompt("Enter Book ID to update", Color::magenta);
    int id; cin >> id; cin.ignore();

    for (auto& b : books) {
        if (b.getId() == id) {
            printBanner("  Editing: " + b.getTitle() + "  (blank = keep current)  ",
                        Color::magenta, false);

            // ── OOP: use setters to update fields ─────────────
            string tmp;
            printPrompt("New Title ["    + b.getTitle()    + "]", Color::magenta); getline(cin, tmp);
            if (!tmp.empty()) b.setTitle(tmp);

            printPrompt("New Author ["   + b.getAuthor()   + "]", Color::magenta); getline(cin, tmp);
            if (!tmp.empty()) b.setAuthor(tmp);

            printPrompt("New Category [" + b.getCategory() + "]", Color::magenta); getline(cin, tmp);
            if (!tmp.empty()) b.setCategory(tmp);

            printPrompt("New Quantity [" + to_string(b.getQuantity()) + "]", Color::magenta);
            getline(cin, tmp);
            if (!tmp.empty()) {
                try { b.setQuantity(stoi(tmp)); } catch (...) {}
            }

            printStatus("Book \"" + b.getTitle() + "\" updated successfully!", true);
            this_thread::sleep_for(chrono::milliseconds(800));
            return;
        }
    }
    printStatus("Book ID " + to_string(id) + " not found.", false);
}

// ═══════════════════════════════════════════════════════════════
//  DELETE BOOK
// ═══════════════════════════════════════════════════════════════
void deleteBook(vector<Book>& books) {
    clearScreen();
    printFigletTitle("DELETE", SP_RED);
    printBanner("  Remove a book from the library  ", Color::red);

    if (books.empty()) { printStatus("No books to delete.", false); return; }
    viewBooks(books);

    printPrompt("Enter Book ID to delete", Color::red);
    int id; cin >> id;

    for (auto it = books.begin(); it != books.end(); ++it) {
        if (it->getId() == id) {
            if (!confirmAction("Delete \"" + it->getTitle() + "\"? Cannot be undone.", Color::red))
            { printStatus("Delete cancelled.", false); return; }
            printStatus("Deleted \"" + it->getTitle() + "\" successfully.", true);
            books.erase(it);
            this_thread::sleep_for(chrono::milliseconds(800));
            return;
        }
    }
    printStatus("Book ID " + to_string(id) + " not found.", false);
}
