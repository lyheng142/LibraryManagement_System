#ifndef BOOK_HPP
#define BOOK_HPP

#include <string>
#include <vector>
using namespace std;

// ═══════════════════════════════════════════════════════════════
//  OOP: Book class with private members and public getters/setters
// ═══════════════════════════════════════════════════════════════
class Book {
private:
    int    id;
    string title;
    string author;
    string category;
    int    quantity;

public:
    // Constructors
    Book();
    Book(int id, const string& title, const string& author,
         const string& category, int quantity);

    // ── Getters ──────────────────────────────────────────────
    int    getId()       const { return id;       }
    string getTitle()    const { return title;    }
    string getAuthor()   const { return author;   }
    string getCategory() const { return category; }
    int    getQuantity() const { return quantity; }

    // ── Setters ──────────────────────────────────────────────
    void setTitle   (const string& t) { title    = t; }
    void setAuthor  (const string& a) { author   = a; }
    void setCategory(const string& c) { category = c; }
    void setQuantity(int q)           { quantity = q; }

    // ── Business logic methods ────────────────────────────────
    bool isAvailable()  const { return quantity > 0;  }
    bool isLowStock()   const { return quantity > 0 && quantity <= 3; }
    bool isOutOfStock() const { return quantity == 0; }
    void decreaseQty()        { if (quantity > 0) quantity--; }
    void increaseQty()        { quantity++; }

    // ── Status label / color ──────────────────────────────────
    string getStatusLabel() const;
};

// ── Free functions for UI (use Book class) ────────────────────
void addBook(vector<Book>& books);
void viewBooks(const vector<Book>& books);
void searchBook(const vector<Book>& books);
void sortBooks(vector<Book>& books);
void updateBook(vector<Book>& books);
void deleteBook(vector<Book>& books);

#endif
