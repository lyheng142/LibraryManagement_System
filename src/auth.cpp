#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <cctype>
#include <filesystem>
#include <tabulate/table.hpp>
#include "auth.hpp"
#include "splash.hpp"
#include "ui_helper.hpp"
#include <OpenXLSX.hpp>

// ── Platform-specific password masking ───────────────────────────────
#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

using namespace std;
using namespace tabulate;
using namespace OpenXLSX;
namespace fs = std::filesystem;

struct User { string username, password, role; };

static string getDataDir() {
    const fs::path cwd = fs::current_path();
    vector<fs::path> candidates = {
        cwd.parent_path() / "data",
        cwd / "data"
    };
    for (const auto& d : candidates)
        if (fs::exists(d) && fs::is_directory(d) &&
            (fs::exists(d/"users.xlsx") || fs::exists(d/"books.xlsx")))
            return d.string();
    for (const auto& d : candidates)
        if (fs::exists(d) && fs::is_directory(d)) return d.string();
    error_code ec;
    fs::create_directories(candidates[0], ec);
    return candidates[0].string();
}

static string usersPath() { return getDataDir() + "/users.xlsx"; }

static string cellStr(XLCell cell) {
    try {
        if (cell.value().type() == XLValueType::String)  return cell.value().get<string>();
        if (cell.value().type() == XLValueType::Integer) return to_string(cell.value().get<int32_t>());
    } catch (...) {}
    return "";
}

// ═══════════════════════════════════════════════════════════════
//  Password masking — works on BOTH Windows and Linux
// ═══════════════════════════════════════════════════════════════
static string readPassword() {
    string pw;
#ifdef _WIN32
    // Windows: use _getch() from conio.h — no echo
    int ch;
    while ((ch = _getch()) != '\r' && ch != '\n') {
        if (ch == '\b' || ch == 127) {
            if (!pw.empty()) { pw.pop_back(); cout << "\b \b" << flush; }
        } else if (ch >= 32) {
            pw += (char)ch;
            cout << '*' << flush;
        }
    }
    cout << "\n";
#else
    // Linux/Mac: disable terminal echo with termios
    termios old, neo;
    tcgetattr(STDIN_FILENO, &old);
    neo = old;
    neo.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
    tcsetattr(STDIN_FILENO, TCSANOW, &neo);
    getline(cin, pw);
    tcsetattr(STDIN_FILENO, TCSANOW, &old);
    cout << "\n";
#endif
    return pw;
}

static Color roleColor(const string& r) {
    return r=="librarian" ? Color::magenta : r=="student" ? Color::cyan : Color::yellow;
}
static string upperRole(string r) {
    for (char& c : r) c=(char)toupper((unsigned char)c); return r;
}

static vector<User> loadUsers() {
    vector<User> users;
    string path = usersPath();
    if (!fs::exists(path)) return users;
    try {
        XLDocument doc; doc.open(path);
        auto wks = doc.workbook().worksheet("Sheet1");
        int row = 2;
        while (wks.cell(XLCellReference(row,1)).value().type() != XLValueType::Empty) {
            User u;
            u.username = cellStr(wks.cell(XLCellReference(row,1)));
            u.password = cellStr(wks.cell(XLCellReference(row,2)));
            u.role     = cellStr(wks.cell(XLCellReference(row,3)));
            if (!u.username.empty()) users.push_back(u);
            row++;
        }
        doc.close();
    } catch (...) {}
    return users;
}

static bool saveUsers(const vector<User>& users) {
    try {
        XLDocument doc; doc.create(usersPath(), true);
        auto wks = doc.workbook().worksheet("Sheet1");
        wks.cell("A1").value() = "Username";
        wks.cell("B1").value() = "Password";
        wks.cell("C1").value() = "Role";
        int row = 2;
        for (const auto& u : users) {
            wks.cell(XLCellReference(row,1)).value() = u.username;
            wks.cell(XLCellReference(row,2)).value() = u.password;
            wks.cell(XLCellReference(row,3)).value() = u.role;
            row++;
        }
        doc.save(); doc.close(); return true;
    } catch (const exception& e) {
        printStatus("Error saving users.xlsx: " + string(e.what()), false);
        return false;
    }
}

bool loginUser(const string& requiredRole, string& loggedInUsername) {
    Color border = roleColor(requiredRole);
    string roleUpper = upperRole(requiredRole);
    printBanner("  " + roleUpper + "  AUTHENTICATION  ", border);

    Table info;
    info.add_row({"Field","Details"});
    info.add_row({"Role", roleUpper});
    info.add_row({"Store","data/users.xlsx"});
    info.add_row({"Hint", "Default: admin/admin123  or  student/1234"});
    info.format().border_color(border).font_color(Color::white);
    info[0].format().font_style({FontStyle::bold}).font_color(Color::yellow).border_bottom_color(border);
    for (size_t i=1;i<info.size();++i){
        info[i][0].format().font_align(FontAlign::right).padding_right(2).font_color(Color::yellow).font_style({FontStyle::bold});
        info[i][1].format().font_color(Color::white);
    }
    info[1][1].format().font_color(border).font_style({FontStyle::bold});
    printCentered(info);

    string username, password;
    printPrompt("Username", border); cin >> username;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    printPrompt("Password", border); password = readPassword();
    cout << "\n";

    if (requiredRole=="librarian" && username=="admin"   && password=="admin123")
        { loggedInUsername=username; printStatus("Welcome, "+username+"! [Default librarian]",true); return true; }
    if (requiredRole=="student"   && username=="student" && password=="1234")
        { loggedInUsername=username; printStatus("Welcome, "+username+"! [Default student]",true); return true; }

    for (const auto& u : loadUsers()) {
        if (u.username==username && u.password==password) {
            if (u.role==requiredRole) {
                loggedInUsername=username;
                printStatus("Login successful  --  Welcome, "+username+"!", true);
                return true;
            }
            printStatus("Wrong role for this account.", false); return false;
        }
    }
    printStatus("Username or password incorrect.", false); return false;
}

bool registerUser(const string& role) {
    Color border = roleColor(role);
    string roleUpper = upperRole(role);
    clearScreen();
    printFigletTitle("REGISTER", role=="librarian" ? SP_MAG : SP_CYAN);
    printBanner("  "+roleUpper+"  REGISTRATION  ", border);

    Table info;
    info.add_row({"Field","Details"});
    info.add_row({"Role",   roleUpper});
    info.add_row({"Save to","data/users.xlsx"});
    info.add_row({"Columns","Username | Password | Role"});
    info.format().border_color(border).font_color(Color::white);
    info[0].format().font_style({FontStyle::bold}).font_color(Color::yellow).border_bottom_color(border);
    for (size_t i=1;i<info.size();++i){
        info[i][0].format().font_align(FontAlign::right).padding_right(2).font_color(Color::yellow).font_style({FontStyle::bold});
        info[i][1].format().font_color(Color::white);
    }
    info[1][1].format().font_color(border).font_style({FontStyle::bold});
    printCentered(info);

    string username, password, confirm;
    printPrompt("New Username",     border); cin >> username;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    printPrompt("New Password",     border); password = readPassword();
    printPrompt("Confirm Password", border); confirm  = readPassword();
    cout << "\n";

    if (username.empty()||password.empty()){ printStatus("Fields cannot be empty.",false); return false; }
    if (password!=confirm){ printStatus("Passwords do not match.",false); return false; }

    auto users = loadUsers();
    for (const auto& u : users)
        if (u.username==username){ printStatus("Username \""+username+"\" already exists.",false); return false; }

    users.push_back({username, password, role});
    if (!saveUsers(users)) return false;

    printCard("SAVED TO users.xlsx",{{"Username",username},{"Password",string(password.size(),'*')},{"Role",role}}, border);
    printStatus("Account saved! You can now login as "+roleUpper+".", true);
    return true;
}
