#ifndef EXCEL_HELPER_HPP
#define EXCEL_HELPER_HPP

// ═══════════════════════════════════════════════════════════════
//  excel_helper.hpp  —  Data Persistency using Excel (.xlsx)
//
//  data/books.xlsx   → all books
//  data/borrow.xlsx  → all borrow records
//  data/users.xlsx   → all user accounts
// ═══════════════════════════════════════════════════════════════

#include <vector>
#include "book.hpp"
#include "borrow.hpp"

void saveBooks  (const std::vector<Book>&   books);
void loadBooks  (      std::vector<Book>&   books);

void saveBorrows(const std::vector<Borrow>& borrows);
void loadBorrows(      std::vector<Borrow>& borrows);

// ── Backup & Restore ──────────────────────────────────────────
bool backupData();                          // backup all Excel files
std::vector<std::string> listBackups();     // list all backups
bool restoreData(const std::string& name);  // restore from backup

#endif
