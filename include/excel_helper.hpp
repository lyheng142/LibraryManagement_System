#ifndef EXCEL_HELPER_HPP
#define EXCEL_HELPER_HPP


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

// ── Session log ───────────────────────────────────────────────
void logSession(const std::string& username, const std::string& role, const std::string& action);
void viewSessionLog();