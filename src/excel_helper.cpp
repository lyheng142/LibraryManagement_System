// ═══════════════════════════════════════════════════════════════
//  excel_helper.cpp  —  ALL data stored in Excel (.xlsx) files
//
//  data/books.xlsx    | A:ID  B:Title  C:Author  D:Category  E:Quantity
//  data/borrow.xlsx   | A:BorrowID  B:Username  C:BookID  D:BookTitle
//                     | E:BorrowDate  F:ReturnDate  G:Returned(0/1)
//  data/users.xlsx    | A:Username  B:Password  C:Role
//                       (managed by auth.cpp)
// ═══════════════════════════════════════════════════════════════
#include <iostream>
#include <filesystem>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <string>
#include <vector>
#include "excel_helper.hpp"
#include <OpenXLSX.hpp>

using namespace std;
using namespace OpenXLSX;
namespace fs = std::filesystem;

// ── Find project's data/ directory ───────────────────────────────────────────
static string getDataDir() {
    const fs::path cwd = fs::current_path();
    vector<fs::path> candidates = {
        cwd.parent_path() / "data",
        cwd / "data"
    };
    // Prefer folder that already has our Excel files
    for (const auto& d : candidates)
        if (fs::exists(d) && fs::is_directory(d) &&
            (fs::exists(d/"books.xlsx") || fs::exists(d/"borrow.xlsx") || fs::exists(d/"users.xlsx")))
            return d.string();
    // Fall back to first existing data/ dir
    for (const auto& d : candidates)
        if (fs::exists(d) && fs::is_directory(d))
            return d.string();
    // Last resort: create it
    error_code ec;
    fs::create_directories(candidates[0], ec);
    return candidates[0].string();
}

static string xlPath(const string& file) {
    return getDataDir() + "/" + file;
}

// ── Safe cell readers ─────────────────────────────────────────────────────────
static int cellInt(XLCell cell) {
    try {
        if (cell.value().type() == XLValueType::Integer) return cell.value().get<int32_t>();
        if (cell.value().type() == XLValueType::Float)   return (int)cell.value().get<double>();
    } catch (...) {}
    return 0;
}

static string cellStr(XLCell cell) {
    try {
        if (cell.value().type() == XLValueType::String)  return cell.value().get<string>();
        if (cell.value().type() == XLValueType::Integer) return to_string(cell.value().get<int32_t>());
    } catch (...) {}
    return "";
}

// ═══════════════════════════════════════════════════════════════
//  BOOKS  ←→  data/books.xlsx
// ═══════════════════════════════════════════════════════════════
void saveBooks(const vector<Book>& books) {
    try {
        XLDocument doc;
        doc.create(xlPath("books.xlsx"), true);
        auto wks = doc.workbook().worksheet("Sheet1");

        wks.cell("A1").value() = "ID";
        wks.cell("B1").value() = "Title";
        wks.cell("C1").value() = "Author";
        wks.cell("D1").value() = "Category";
        wks.cell("E1").value() = "Quantity";

        int row = 2;
        for (const auto& b : books) {
            wks.cell(XLCellReference(row,1)).value() = b.getId();
            wks.cell(XLCellReference(row,2)).value() = b.getTitle();
            wks.cell(XLCellReference(row,3)).value() = b.getAuthor();
            wks.cell(XLCellReference(row,4)).value() = b.getCategory();
            wks.cell(XLCellReference(row,5)).value() = b.getQuantity();
            row++;
        }
        doc.save();
        doc.close();
    } catch (const exception& e) {
        cerr << "[Excel] Error saving books.xlsx: " << e.what() << "\n";
    }
}

void loadBooks(vector<Book>& books) {
    string path = xlPath("books.xlsx");
    if (!fs::exists(path)) return;
    try {
        XLDocument doc;
        doc.open(path);
        auto wks = doc.workbook().worksheet("Sheet1");
        books.clear();
        int row = 2;
        while (wks.cell(XLCellReference(row,1)).value().type() != XLValueType::Empty) {
            books.emplace_back(
                cellInt(wks.cell(XLCellReference(row,1))),
                cellStr(wks.cell(XLCellReference(row,2))),
                cellStr(wks.cell(XLCellReference(row,3))),
                cellStr(wks.cell(XLCellReference(row,4))),
                cellInt(wks.cell(XLCellReference(row,5)))
            );
            row++;
        }
        doc.close();
    } catch (const exception& e) {
        cerr << "[Excel] Error loading books.xlsx: " << e.what() << "\n";
    }
}

// ═══════════════════════════════════════════════════════════════
//  BORROWS  ←→  data/borrow.xlsx
// ═══════════════════════════════════════════════════════════════
void saveBorrows(const vector<Borrow>& borrows) {
    try {
        XLDocument doc;
        doc.create(xlPath("borrow.xlsx"), true);
        auto wks = doc.workbook().worksheet("Sheet1");

        wks.cell("A1").value() = "BorrowID";
        wks.cell("B1").value() = "Username";
        wks.cell("C1").value() = "BookID";
        wks.cell("D1").value() = "BookTitle";
        wks.cell("E1").value() = "BorrowDate";
        wks.cell("F1").value() = "ReturnDate";
        wks.cell("G1").value() = "Returned";

        int row = 2;
        for (const auto& b : borrows) {
            wks.cell(XLCellReference(row,1)).value() = b.getBorrowId();
            wks.cell(XLCellReference(row,2)).value() = b.getUsername();
            wks.cell(XLCellReference(row,3)).value() = b.getBookId();
            wks.cell(XLCellReference(row,4)).value() = b.getBookTitle();
            wks.cell(XLCellReference(row,5)).value() = b.getBorrowDate();
            wks.cell(XLCellReference(row,6)).value() = b.getReturnDate();
            wks.cell(XLCellReference(row,7)).value() = b.isReturned() ? 1 : 0;
            row++;
        }
        doc.save();
        doc.close();
    } catch (const exception& e) {
        cerr << "[Excel] Error saving borrow.xlsx: " << e.what() << "\n";
    }
}

void loadBorrows(vector<Borrow>& borrows) {
    string path = xlPath("borrow.xlsx");
    if (!fs::exists(path)) return;
    try {
        XLDocument doc;
        doc.open(path);
        auto wks = doc.workbook().worksheet("Sheet1");
        borrows.clear();
        int row = 2;
        while (wks.cell(XLCellReference(row,1)).value().type() != XLValueType::Empty) {
            Borrow b(
                cellInt(wks.cell(XLCellReference(row,1))),
                cellStr(wks.cell(XLCellReference(row,2))),
                cellInt(wks.cell(XLCellReference(row,3))),
                cellStr(wks.cell(XLCellReference(row,4))),
                cellStr(wks.cell(XLCellReference(row,5)))
            );
            if (cellInt(wks.cell(XLCellReference(row,7))) == 1)
                b.markReturned(cellStr(wks.cell(XLCellReference(row,6))));
            borrows.push_back(b);
            row++;
        }
        doc.close();
    } catch (const exception& e) {
        cerr << "[Excel] Error loading borrow.xlsx: " << e.what() << "\n";
    }
}

// ═══════════════════════════════════════════════════════════════
//  BACKUP & RESTORE  ←→  data/backup/
//
//  Backup creates a folder: data/backup/YYYY-MM-DD_HH-MM-SS/
//  and copies books.xlsx, borrow.xlsx, users.xlsx into it
// ═══════════════════════════════════════════════════════════════

// Get current datetime as string for folder name
static string getBackupTimestamp() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    ostringstream ss;
    ss << (1900 + ltm->tm_year)
       << "-" << setw(2) << setfill('0') << (1 + ltm->tm_mon)
       << "-" << setw(2) << setfill('0') << ltm->tm_mday
       << "_" << setw(2) << setfill('0') << ltm->tm_hour
       << "-" << setw(2) << setfill('0') << ltm->tm_min
       << "-" << setw(2) << setfill('0') << ltm->tm_sec;
    return ss.str();
}

// Create a backup of all 3 Excel files
bool backupData() {
    try {
        string dataDir   = getDataDir();
        string backupDir = dataDir + "/backup";
        string stamp     = getBackupTimestamp();
        string destDir   = backupDir + "/" + stamp;

        // Create backup folder
        error_code ec;
        fs::create_directories(destDir, ec);
        if (ec) {
            cerr << "[Backup] Cannot create folder: " << destDir << "\n";
            return false;
        }

        // Copy each Excel file if it exists
        vector<string> files = {"books.xlsx", "borrow.xlsx", "users.xlsx"};
        int copied = 0;
        for (const auto& f : files) {
            string src = dataDir + "/" + f;
            string dst = destDir + "/" + f;
            if (fs::exists(src)) {
                fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
                if (!ec) copied++;
            }
        }

        if (copied == 0) {
            cerr << "[Backup] No files to backup!\n";
            return false;
        }

        cout << "[Backup] Saved to: " << destDir << "\n";
        cout << "[Backup] " << copied << " file(s) backed up.\n";
        return true;

    } catch (const exception& e) {
        cerr << "[Backup] Error: " << e.what() << "\n";
        return false;
    }
}

// List all available backups
vector<string> listBackups() {
    vector<string> backups;
    string backupDir = getDataDir() + "/backup";

    if (!fs::exists(backupDir)) return backups;

    for (const auto& entry : fs::directory_iterator(backupDir)) {
        if (fs::is_directory(entry.path())) {
            backups.push_back(entry.path().filename().string());
        }
    }

    // Sort newest first
    sort(backups.rbegin(), backups.rend());
    return backups;
}

// Restore from a specific backup folder name
bool restoreData(const string& backupName) {
    try {
        string dataDir   = getDataDir();
        string srcDir    = dataDir + "/backup/" + backupName;

        if (!fs::exists(srcDir)) {
            cerr << "[Restore] Backup not found: " << backupName << "\n";
            return false;
        }

        vector<string> files = {"books.xlsx", "borrow.xlsx", "users.xlsx"};
        int restored = 0;
        error_code ec;

        for (const auto& f : files) {
            string src = srcDir + "/" + f;
            string dst = dataDir + "/" + f;
            if (fs::exists(src)) {
                fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
                if (!ec) restored++;
            }
        }

        cout << "[Restore] Restored " << restored << " file(s) from: " << backupName << "\n";
        return restored > 0;

    } catch (const exception& e) {
        cerr << "[Restore] Error: " << e.what() << "\n";
        return false;
    }
}
