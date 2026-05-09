#ifndef BORROW_HPP
#define BORROW_HPP

#include <string>
#include <vector>
#include "book.hpp"
using namespace std;

// ═══════════════════════════════════════════════════════════════
//  OOP: Borrow class with private members and public getters/setters
// ═══════════════════════════════════════════════════════════════
class Borrow {
private:
    int    borrowId;
    string username;
    int    bookId;
    string bookTitle;
    string borrowDate;
    string returnDate;
    bool   returned;

public:
    // Constructors
    Borrow();
    Borrow(int borrowId, const string& username, int bookId,
           const string& bookTitle, const string& borrowDate);

    // ── Getters ──────────────────────────────────────────────
    int    getBorrowId()   const { return borrowId;   }
    string getUsername()   const { return username;   }
    int    getBookId()     const { return bookId;     }
    string getBookTitle()  const { return bookTitle;  }
    string getBorrowDate() const { return borrowDate; }
    string getReturnDate() const { return returnDate; }
    bool   isReturned()    const { return returned;   }

    // ── Business logic methods ────────────────────────────────
    void markReturned(const string& date);
    bool isActive()    const { return !returned; }
    bool belongsTo(const string& user) const { return username == user; }
};

// ── Free functions for UI ─────────────────────────────────────
void borrowBook  (vector<Book>& books, vector<Borrow>& borrows, const string& username);
void returnBook  (vector<Book>& books, vector<Borrow>& borrows, const string& username);
void viewBorrowHistory(const vector<Borrow>& borrows, const string& username = "");
void viewOverdueBooks (const vector<Borrow>& borrows);

#endif
