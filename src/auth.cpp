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
#include <ctime>
#include <sstream>
#include <iomanip>
#include <array>
#include <OpenXLSX.hpp>

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
    vector<fs::path> candidates = { cwd.parent_path()/"data", cwd/"data" };
    for (const auto& d : candidates)
        if (fs::exists(d) && fs::is_directory(d) &&
            (fs::exists(d/"users.xlsx") || fs::exists(d/"books.xlsx")))
            return d.string();
    for (const auto& d : candidates)
        if (fs::exists(d) && fs::is_directory(d)) return d.string();
    error_code ec; fs::create_directories(candidates[0], ec);
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

static string readPassword() {
    string pw;
#ifdef _WIN32
    int ch;
    while ((ch = _getch()) != '\r' && ch != '\n') {
        if (ch == '\b' || ch == 127) { if (!pw.empty()) { pw.pop_back(); cout << "\b \b" << flush; } }
        else if (ch >= 32) { pw += (char)ch; cout << '*' << flush; }
    }
    cout << "\n";
#else
    termios old, neo;
    tcgetattr(STDIN_FILENO, &old); neo = old;
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
static string upperRole(string r) { for (char& c:r) c=(char)toupper((unsigned char)c); return r; }

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
        printStatus("Error saving users.xlsx: "+string(e.what()), false);
        return false;
    }
}

bool loginUser(const string& requiredRole, string& loggedInUsername) {
    Color border = roleColor(requiredRole);
    string roleUpper = upperRole(requiredRole);
    printBanner("  "+roleUpper+"  AUTHENTICATION  ", border);
    Table info;
    info.add_row({"Field","Details"});
    info.add_row({"Role", roleUpper});
    info.add_row({"Store","data/users.xlsx"});
    info.add_row({"Hint", "Default: admin/admin123  or  student/1234"});
    info.format().border_color(border).font_color(Color::white);
    info[0].format().font_style({FontStyle::bold}).font_color(Color::yellow).border_bottom_color(border);
    for (size_t i=1;i<info.size();++i) {
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
            if (u.role==requiredRole) { loggedInUsername=username; printStatus("Login successful  --  Welcome, "+username+"!", true); return true; }
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
    for (size_t i=1;i<info.size();++i) {
        info[i][0].format().font_align(FontAlign::right).padding_right(2).font_color(Color::yellow).font_style({FontStyle::bold});
        info[i][1].format().font_color(Color::white);
    }
    info[1][1].format().font_color(border).font_style({FontStyle::bold});
    printCentered(info);
    string username, password, confirm;
    printPrompt("New Username", border); cin >> username;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    printPrompt("New Password", border); password = readPassword();
    printPrompt("Confirm Password", border); confirm = readPassword();
    cout << "\n";
    username.erase(0, username.find_first_not_of(" \t"));
    username.erase(username.find_last_not_of(" \t") + 1);
    if (username.empty())      { printStatus("Username cannot be empty.", false); return false; }
    if (username.size() < 3)   { printStatus("Username must be at least 3 characters.", false); return false; }
    if (username.size() > 30)  { printStatus("Username too long (max 30 chars).", false); return false; }
    if (password.empty())      { printStatus("Password cannot be empty.", false); return false; }
    if (password.size() < 4)   { printStatus("Password must be at least 4 characters.", false); return false; }
    if (password != confirm)   { printStatus("Passwords do not match.", false); return false; }
    if (username.find(' ') != string::npos) { printStatus("Username cannot contain spaces.", false); return false; }
    auto users = loadUsers();
    for (const auto& u : users)
        if (u.username==username) { printStatus("Username \""+username+"\" already exists.", false); return false; }
    users.push_back({username, password, role});
    if (!saveUsers(users)) return false;
    printCard("SAVED TO users.xlsx",{{"Username",username},{"Password",string(password.size(),'*')},{"Role",role}}, border);
    printStatus("Account saved! You can now login as "+roleUpper+".", true);
    return true;
}

void manageUsers() {
    clearScreen();
    printFigletTitle("MANAGE", SP_MAG);
    printFigletTitle("USERS", SP_MAG);
    printBanner("  View and delete registered accounts  ", Color::magenta);
    auto users = loadUsers();
    if (users.empty()) { printStatus("No registered users found in users.xlsx.", false); return; }
    Table t;
    t.add_row({"No", "Username", "Role"});
    for (int i=0;i<(int)users.size();i++)
        t.add_row({to_string(i+1), users[i].username, users[i].role});
    t[0].format().font_style({FontStyle::bold}).font_color(Color::yellow).border_bottom_color(Color::magenta);
    t.format().border_color(Color::magenta);
    for (size_t i=1;i<t.size();i++) {
        Color rc = users[i-1].role=="librarian" ? Color::magenta : Color::cyan;
        t[i][2].format().font_color(rc).font_style({FontStyle::bold});
    }
    for (size_t i=0;i<t.size();i++) {
        t[i][0].format().width(5).font_align(FontAlign::center);
        t[i][1].format().width(20).font_align(FontAlign::left);
        t[i][2].format().width(12).font_align(FontAlign::left);
    }
    printCenteredBlankLine(); printCentered(t);
    printCard("TOTAL USERS",{{"Registered",to_string(users.size())},{"Note","Default accounts cannot be deleted"}}, Color::magenta);
    printPrompt("Enter user number to DELETE (0 to cancel)", Color::red);
    int choice; cin >> choice; cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (choice<=0||choice>(int)users.size()) { printStatus("Cancelled. No users deleted.", false); return; }
    User& target = users[choice-1];
    if ((target.username=="admin"&&target.role=="librarian")||(target.username=="student"&&target.role=="student"))
        { printStatus("Cannot delete default built-in accounts!", false); return; }
    printBanner("  You are about to delete: "+target.username+" ("+target.role+")  ", Color::red);
    printPrompt("Type the username to confirm deletion", Color::red);
    string confirm; getline(cin, confirm);
    if (confirm != target.username) { printStatus("Username did not match. Deletion cancelled.", false); return; }
    string deletedName = target.username;
    users.erase(users.begin()+(choice-1));
    if (saveUsers(users)) printStatus("User \""+deletedName+"\" deleted successfully!", true);
    else printStatus("Failed to save changes.", false);
}

bool changePassword(const string& username, const string& role) {
    Color border = roleColor(role);
    clearScreen();
    printFigletTitle("CHANGE", role=="librarian" ? SP_MAG : SP_CYAN);
    printFigletTitle("PASSWORD", role=="librarian" ? SP_MAG : SP_CYAN);
    printBanner("  Update your login password  ", border);
    if ((username=="admin"&&role=="librarian")||(username=="student"&&role=="student"))
        { printStatus("Default built-in accounts cannot change their password.", false); return false; }
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    printPrompt("Current password", border);
    string current = readPassword();
    auto users = loadUsers();
    bool found=false; int idx=-1;
    for (int i=0;i<(int)users.size();i++) {
        if (users[i].username==username&&users[i].role==role) {
            found=true; idx=i;
            if (users[i].password!=current) { printStatus("Current password is incorrect.", false); return false; }
            break;
        }
    }
    if (!found) { printStatus("Account not found in users.xlsx.", false); return false; }
    printPrompt("New password", border); string newPass=readPassword();
    printPrompt("Confirm new password", border); string confirm=readPassword();
    cout << "\n";
    if (newPass.empty())    { printStatus("New password cannot be empty.", false); return false; }
    if (newPass.size()<4)   { printStatus("Password must be at least 4 characters.", false); return false; }
    if (newPass==current)   { printStatus("New password must differ from current one.", false); return false; }
    if (newPass!=confirm)   { printStatus("Passwords do not match.", false); return false; }
    users[idx].password = newPass;
    if (!saveUsers(users)) return false;
    printCard("PASSWORD UPDATED",{{"Username",username},{"Role",role},{"Status","Password changed successfully!"}}, border);
    printStatus("Next login will use your new password.", true);
    return true;
}
// ═══════════════════════════════════════════════════════════════
//  SESSION LOG  ←→  data/session_log.xlsx
//  Tracks every login and logout with timestamp
// ═══════════════════════════════════════════════════════════════

static string sessionLogPath() { return getDataDir() + "/session_log.xlsx"; }

static string getNow() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    ostringstream ss;
    ss << (1900+ltm->tm_year)
       << "-" << setw(2) << setfill('0') << (1+ltm->tm_mon)
       << "-" << setw(2) << setfill('0') << ltm->tm_mday
       << " " << setw(2) << setfill('0') << ltm->tm_hour
       << ":" << setw(2) << setfill('0') << ltm->tm_min
       << ":" << setw(2) << setfill('0') << ltm->tm_sec;
    return ss.str();
}

void logSession(const string& username, const string& role, const string& action) {
    string path = sessionLogPath();
    int nextRow = 2;
    vector<array<string,4>> existing;

    // Load existing rows first
    if (fs::exists(path)) {
        try {
            XLDocument doc; doc.open(path);
            auto wks = doc.workbook().worksheet("Sheet1");
            int r = 2;
            while (wks.cell(XLCellReference(r,1)).value().type() != XLValueType::Empty) {
                array<string,4> row;
                row[0] = cellStr(wks.cell(XLCellReference(r,1)));
                row[1] = cellStr(wks.cell(XLCellReference(r,2)));
                row[2] = cellStr(wks.cell(XLCellReference(r,3)));
                row[3] = cellStr(wks.cell(XLCellReference(r,4)));
                existing.push_back(row);
                r++;
            }
            doc.close();
            nextRow = r;
        } catch (...) {}
    }

    // Append new entry
    try {
        XLDocument doc;
        if (fs::exists(path)) doc.open(path);
        else {
            doc.create(path, true);
            auto wks = doc.workbook().worksheet("Sheet1");
            wks.cell("A1").value() = "Timestamp";
            wks.cell("B1").value() = "Username";
            wks.cell("C1").value() = "Role";
            wks.cell("D1").value() = "Action";
        }
        auto wks = doc.workbook().worksheet("Sheet1");
        wks.cell("A1").value() = "Timestamp";
        wks.cell("B1").value() = "Username";
        wks.cell("C1").value() = "Role";
        wks.cell("D1").value() = "Action";

        // Rewrite all existing rows
        int r = 2;
        for (const auto& row : existing) {
            wks.cell(XLCellReference(r,1)).value() = row[0];
            wks.cell(XLCellReference(r,2)).value() = row[1];
            wks.cell(XLCellReference(r,3)).value() = row[2];
            wks.cell(XLCellReference(r,4)).value() = row[3];
            r++;
        }
        // Add new row
        wks.cell(XLCellReference(r,1)).value() = getNow();
        wks.cell(XLCellReference(r,2)).value() = username;
        wks.cell(XLCellReference(r,3)).value() = role;
        wks.cell(XLCellReference(r,4)).value() = action;

        doc.save(); doc.close();
    } catch (...) {}
}

void viewSessionLog() {
    clearScreen();
    printFigletTitle("SESSION", SP_CYAN);
    printFigletTitle("LOG", SP_CYAN);
    printBanner("  View all login and logout activity  ", Color::cyan);

    string path = sessionLogPath();
    if (!fs::exists(path)) {
        printStatus("No session log found. No one has logged in yet.", false);
        return;
    }

    vector<array<string,4>> rows;
    try {
        XLDocument doc; doc.open(path);
        auto wks = doc.workbook().worksheet("Sheet1");
        int r = 2;
        while (wks.cell(XLCellReference(r,1)).value().type() != XLValueType::Empty) {
            array<string,4> row;
            row[0] = cellStr(wks.cell(XLCellReference(r,1)));
            row[1] = cellStr(wks.cell(XLCellReference(r,2)));
            row[2] = cellStr(wks.cell(XLCellReference(r,3)));
            row[3] = cellStr(wks.cell(XLCellReference(r,4)));
            rows.push_back(row);
            r++;
        }
        doc.close();
    } catch (...) {
        printStatus("Error reading session_log.xlsx.", false); return;
    }

    if (rows.empty()) { printStatus("Session log is empty.", false); return; }

    // Show with pagination using tabulate
    Table t;
    t.add_row({"#", "Timestamp", "Username", "Role", "Action"});
    for (int i = 0; i < (int)rows.size(); i++) {
        string actionColor = rows[i][3] == "LOGIN" ? "LOGIN" : "LOGOUT";
        t.add_row({to_string(i+1), rows[i][0], rows[i][1], rows[i][2], rows[i][3]});
    }
    t[0].format().font_style({FontStyle::bold}).font_color(Color::yellow)
        .border_bottom_color(Color::cyan);
    t.format().border_color(Color::cyan);
    for (size_t i = 1; i < t.size(); i++) {
        t[i].format().font_color(i%2==1 ? Color::cyan : Color::white);
        bool isLogin = rows[i-1][3] == "LOGIN";
        t[i][4].format().font_color(isLogin ? Color::green : Color::red)
            .font_style({FontStyle::bold});
    }
    for (size_t i = 0; i < t.size(); i++) {
        t[i][0].format().width(4).font_align(FontAlign::center);
        t[i][1].format().width(20).font_align(FontAlign::center);
        t[i][2].format().width(14).font_align(FontAlign::left);
        t[i][3].format().width(12).font_align(FontAlign::left);
        t[i][4].format().width(8).font_align(FontAlign::center);
    }
    printCenteredBlankLine(); printCentered(t);
    printCard("SUMMARY", {
        {"Total Records", to_string(rows.size())},
        {"Saved to", "data/session_log.xlsx"}
    }, Color::cyan);
}