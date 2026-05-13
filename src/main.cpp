#include <iostream>
#include <vector>
#include <clocale>
#include <locale>
#include <cstdlib>

#ifdef _WIN32
struct _LocaleFix {
    _LocaleFix() { _putenv("LC_ALL=C"); _putenv("LANG=C"); _putenv("LANGUAGE=C"); setlocale(LC_ALL,"C"); }
} _locale_fix_instance;
#endif
#include <limits>
#include <string>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cstdint>
#include "book.hpp"
#include "borrow.hpp"
#include "excel_helper.hpp"
#include "auth.hpp"
#include "splash.hpp"
#include "ui_helper.hpp"

using namespace std;
using namespace tabulate;

vector<Book>   books;
vector<Borrow> borrows;

static void clearInput() { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); }

static void pressEnter() {
    Table t; t.add_row({"  Press ENTER to continue...  "});
    t[0].format().font_align(FontAlign::center).font_color(Color::white)
        .border_color(Color::white).padding_top(0).padding_bottom(0).width(TBL_W);
    printCenteredBlankLine(); printCentered(t);
    cin.ignore(numeric_limits<streamsize>::max(),'\n'); cin.get();
}

static void syncFromDisk() { loadBooks(books); loadBorrows(borrows); }

static uint32_t visW(const string& s) {
    uint32_t w=0; size_t i=0;
    while(i<s.size()){
        unsigned char c=(unsigned char)s[i];
        if(c==0x1B&&i+1<s.size()&&s[i+1]=='['){i+=2;while(i<s.size()&&s[i]!='m')i++;i++;continue;}
        if(c<0x80){w++;i++;continue;}
        int bytes=1; uint32_t cp=0;
        if((c&0xE0)==0xC0){bytes=2;cp=c&0x1F;}
        else if((c&0xF0)==0xE0){bytes=3;cp=c&0x0F;}
        else if((c&0xF8)==0xF0){bytes=4;cp=c&0x07;}
        else{i++;continue;}
        for(int j=1;j<bytes&&i+j<s.size();j++) cp=(cp<<6)|((unsigned char)s[i+j]&0x3F);
        i+=bytes;
        if(cp==0xFE0F||cp==0x200D||(cp>=0x1F3FB&&cp<=0x1F3FF))continue;
        if(cp>=0x1F300||(cp>=0x2600&&cp<=0x27FF)){w+=2;continue;}
        w++;
    }
    return w;
}
static string rep(const string& s,int n){string r;for(int i=0;i<n;i++)r+=s;return r;}
static const char* ac(Color c){
    switch(c){case Color::magenta:return SP_MAG;case Color::cyan:return SP_CYAN;
    case Color::yellow:return SP_YELLOW;case Color::green:return SP_GREEN;
    case Color::red:return SP_RED;case Color::blue:return SP_BLUE;default:return SP_WHITE;}
}
static const char* RB[]={SP_RED,SP_YELLOW,SP_GREEN,SP_CYAN,SP_BLUE,SP_MAG,"\033[35;1m"};
static const int RB_N=7;

static string icon(const string& l){
    string lo=l; transform(lo.begin(),lo.end(),lo.begin(),[](unsigned char c){return tolower(c);});
    if(lo.find("add")      !=string::npos) return u8"➕";
    if(lo.find("view")     !=string::npos) return u8"📋";
    if(lo.find("search")   !=string::npos) return u8"🔍";
    if(lo.find("sort")     !=string::npos) return u8"🔃";
    if(lo.find("update")   !=string::npos) return u8"✏️ ";
    if(lo.find("delete")   !=string::npos) return u8"🗑️ ";
    if(lo.find("borrow")   !=string::npos) return u8"📖";
    if(lo.find("return")   !=string::npos) return u8"↩️ ";
    if(lo.find("history")  !=string::npos) return u8"📜";
    if(lo.find("overdue")  !=string::npos) return u8"⚠️ ";
    if(lo.find("session")  !=string::npos) return u8"🕐";
    if(lo.find("sign out") !=string::npos||lo.find("logout")!=string::npos) return u8"🚪";
    if(lo.find("backup")   !=string::npos) return u8"💾";
    if(lo.find("restore")  !=string::npos) return u8"♻️ ";
    if(lo.find("manage")   !=string::npos) return u8"🔧";
    if(lo.find("change")   !=string::npos||lo.find("password")!=string::npos) return u8"🔑";
    if(lo.find("back")     !=string::npos) return u8"🔙";
    if(lo.find("exit")     !=string::npos) return u8"🚪";
    if(lo.find("login")    !=string::npos) return u8"🔐";
    if(lo.find("register") !=string::npos) return u8"📝";
    if(lo.find("librarian")!=string::npos) return u8"📚";
    return u8"⭐";
}
static bool isExit(const string& l){
    string lo=l; transform(lo.begin(),lo.end(),lo.begin(),[](unsigned char c){return tolower(c);});
    return lo.find("back")!=string::npos||lo.find("exit")!=string::npos||
           lo.find("sign out")!=string::npos||lo.find("logout")!=string::npos;
}

static void printDashMenu(const string& title, const vector<pair<string,string>>& opts, Color borderCol) {
    (void)borderCol;
    const char* RST=SP_RESET;
    const int NO=6,OP=49,IW=56;
    const string H=u8"─";
    const string pad(centeredLeftPad(58),' ');
    const int rb=(int)centeredLeftPad(58)+58;
    auto bl=[&](const string& s,const char* col){ cout<<pad<<col<<SP_BOLD<<s<<RST<<"\n"; };
    auto cr=[&](const string& nr,const string& oc,const char* textCol,const char* leftBC,const char* rightBC,bool bold){
        int nw=visW(nr); string nc=string(NO-nw-1,' ')+nr+" ";
        cout<<pad<<leftBC<<SP_BOLD<<u8"│"<<RST<<SP_WHITE<<SP_BOLD<<nc<<RST
            <<leftBC<<SP_BOLD<<u8"│"<<RST<<textCol<<(bold?SP_BOLD:"")<<oc<<RST;
        cout<<"\033["<<rb<<"G"<<rightBC<<SP_BOLD<<u8"│"<<RST<<"\n";
    };
    string top=u8"┌"+rep(H,IW)+u8"┐";
    string ts =u8"├"+rep(H,NO)+u8"┬"+rep(H,OP)+u8"┤";
    string hs =u8"├"+rep(H,NO)+u8"┼"+rep(H,OP)+u8"┤";
    string bot=u8"└"+rep(H,NO)+u8"┴"+rep(H,OP)+u8"┘";
    printCenteredBlankLine(); bl(top,RB[0]);
    int tw=visW(title),lp=(IW-tw)/2,rp=IW-tw-lp;
    cout<<pad<<RB[0]<<SP_BOLD<<u8"│"<<RST<<RB[0]<<SP_BOLD<<string(lp,' ')<<title<<string(rp,' ')<<RST<<RB[0]<<SP_BOLD<<u8"│"<<RST<<"\n";
    bl(ts,RB[1%RB_N]); cr("No"," Options",SP_YELLOW,RB[1%RB_N],RB[1%RB_N],true);
    for(size_t i=0;i<opts.size();i++){
        const char* rowBC=RB[(i+2)%RB_N];
        bl(hs,rowBC);
        string ic=icon(opts[i].second), tx=" "+ic+"  "+opts[i].second;
        const char* textCol=isExit(opts[i].second)?SP_RED:rowBC;
        cr(to_string(i+1),tx,textCol,rowBC,rowBC,true);
    }
    bl(bot,RB[(opts.size()+2)%RB_N]);
    cout<<pad<<SP_WHITE<<SP_BOLD<<">> Choose (1-"<<opts.size()<<"): "<<SP_RESET;
}

static void printLibrarianStats() {
    int titles=0,copies=0,outStock=0,active=0;
    for(const auto& b:books){titles++;copies+=b.getQuantity();if(b.isOutOfStock())outStock++;}
    for(const auto& br:borrows) if(br.isActive()) active++;
    printCard("LIBRARY STATUS",{{"Total Titles",to_string(titles)},{"Total Copies",to_string(copies)},
        {"Out of Stock",to_string(outStock)},{"Active Borrows",to_string(active)}},Color::magenta);
}

static void printStudentStats(const string& username) {
    int myTotal=0,myActive=0;
    for(const auto& br:borrows){if(br.belongsTo(username)){myTotal++;if(br.isActive())myActive++;}}
    printCard("MY ACCOUNT",{{"Username",username},{"My Borrows",to_string(myTotal)},{"Active",to_string(myActive)}},Color::cyan);
}

static void doBackup() {
    clearScreen(); printFigletTitle("BACKUP",SP_GREEN);
    printBanner("  Save a backup of all Excel data  ",Color::green);
    printStatus(backupData()?"Backup created successfully!":"Backup failed!", backupData());
}

static void doRestore(vector<Book>& books,vector<Borrow>& borrows) {
    clearScreen(); printFigletTitle("RESTORE",SP_YELLOW);
    printBanner("  Restore data from a backup  ",Color::yellow);
    vector<string> backups=listBackups();
    if(backups.empty()){printStatus("No backups found! Create a backup first.",false);return;}
    Table t; t.add_row({"No","Backup Date & Time"});
    for(int i=0;i<(int)backups.size();i++) t.add_row({to_string(i+1),backups[i]});
    t[0].format().font_style({FontStyle::bold}).font_color(Color::yellow).border_bottom_color(Color::yellow);
    t.format().border_color(Color::yellow);
    for(size_t i=0;i<t.size();i++){t[i][0].format().width(5).font_align(FontAlign::center);t[i][1].format().width(25);}
    printCenteredBlankLine(); printCentered(t);
    printPrompt("Choose backup number (0 to cancel)",Color::yellow);
    int choice; cin>>choice;
    if(choice<=0||choice>(int)backups.size()){printStatus("Restore cancelled.",false);return;}
    string selected=backups[choice-1];
    if(!confirmAction("Restore from: "+selected+"? Current data will be replaced!",Color::yellow))
        {printStatus("Restore cancelled.",false);return;}
    if(restoreData(selected)){loadBooks(books);loadBorrows(borrows);printStatus("Data restored successfully from: "+selected,true);}
    else printStatus("Restore failed!",false);
}

void librarianMenu(const string& username) {
    int choice;
    do {
        syncFromDisk(); clearScreen();
        printFigletTitle("LIBRARIAN",SP_MAG);
        printLibrarianStats();
        printDashMenu("Librarian Options",{
            {"[1]",  "Add Book"},
            {"[2]",  "View Books"},
            {"[3]",  "Sort Books"},
            {"[4]",  "Search / Filter"},
            {"[5]",  "Update Book"},
            {"[6]",  "Delete Book"},
            {"[7]",  "View All Borrow History"},
            {"[8]",  "View Overdue Books"},
            {"[9]",  "Manage Users"},
            {"[10]", "Change Password"},
            {"[11]", "View Session Log"},
            {"[12]", "Backup Data"},
            {"[13]", "Restore Data"},
            {"[14]", "Sign Out / Logout"}
        },Color::magenta);
        cin>>choice;
        cin.ignore(numeric_limits<streamsize>::max(),'\n');
        if(cin.fail()){clearInput();printStatus("Invalid input!",false);continue;}
        switch(choice){
            case 1:  addBook(books);    saveBooks(books);    break;
            case 2:  viewBooks(books);                       break;
            case 3:  sortBooks(books);  saveBooks(books);    break;
            case 4:  searchBook(books);                      break;
            case 5:  updateBook(books); saveBooks(books);    break;
            case 6:  deleteBook(books); saveBooks(books);    break;
            case 7:  viewBorrowHistory(borrows,"");          break;
            case 8:  viewOverdueBooks(borrows);              break;
            case 9:  manageUsers();                          break;
            case 10: changePassword(username,"librarian");   break;
            case 11: viewSessionLog();                       break;
            case 12: doBackup();                             break;
            case 13: doRestore(books,borrows);               break;
            case 14:
                logSession(username,"librarian","LOGOUT");   // ← log logout
                clearScreen(); printFigletTitle("SIGN OUT",SP_MAG);
                printStatus("Logged out successfully. See you next time!",true);
                this_thread::sleep_for(chrono::milliseconds(1000)); return;
            default: printStatus("Invalid choice! Choose 1-14.",false);
        }
        if(choice>=1&&choice<=13) pressEnter();
    } while(choice!=14);
}

void studentMenu(const string& username) {
    int choice;
    do {
        syncFromDisk(); clearScreen();
        printFigletTitle("STUDENT",SP_CYAN);
        printStudentStats(username);
        printDashMenu("Student Options",{
            {"[1]", "View Books"},
            {"[2]", "Search / Filter"},
            {"[3]", "Sort Books"},
            {"[4]", "Borrow Book"},
            {"[5]", "Return Book"},
            {"[6]", "My Borrow History"},
            {"[7]", "Change Password"},
            {"[8]", "Sign Out / Logout"}
        },Color::cyan);
        cin>>choice;
        cin.ignore(numeric_limits<streamsize>::max(),'\n');
        if(cin.fail()){clearInput();printStatus("Invalid input!",false);continue;}
        switch(choice){
            case 1: viewBooks(books);                               break;
            case 2: searchBook(books);                              break;
            case 3: sortBooks(books);                               break;
            case 4: borrowBook(books,borrows,username); saveBooks(books); saveBorrows(borrows); break;
            case 5: returnBook(books,borrows,username); saveBooks(books); saveBorrows(borrows); break;
            case 6: viewBorrowHistory(borrows,username);            break;
            case 7: changePassword(username,"student");             break;
            case 8:
                logSession(username,"student","LOGOUT");            // ← log logout
                clearScreen(); printFigletTitle("SIGN OUT",SP_CYAN);
                printStatus("Logged out. Goodbye, "+username+"!",true);
                this_thread::sleep_for(chrono::milliseconds(1000)); return;
            default: printStatus("Invalid choice! Choose 1-8.",false);
        }
        if(choice>=1&&choice<=7) pressEnter();
    } while(choice!=8);
}

static string chooseRole(const string& heading,Color bc) {
    int ch;
    do {
        clearScreen(); printFigletTitle("ACCOUNT",SP_MAG); printFigletTitle("PORTAL",SP_MAG);
        printBanner("  "+heading+"  --  Select Role  ",bc,false);
        printDashMenu("Role Options",{{"[1]","Librarian Account"},{"[2]","Student Account"},{"[3]","Back"}},bc);
        cin>>ch; if(cin.fail()){clearInput();continue;}
        if(ch==1) return "librarian"; if(ch==2) return "student"; if(ch==3) return "";
    } while(true);
}

static bool doLogin(const string& role,string& outUser);

static pair<string,string> runAccessPortal() {
    int ch;
    do {
        clearScreen(); printFigletTitle("ACCESS",SP_MAG); printFigletTitle("PORTAL",SP_MAG);
        Table note; note.add_row({"  Accounts saved to data/users.xlsx  "});
        note[0].format().font_align(FontAlign::center).font_style({FontStyle::bold})
            .font_color(Color::magenta).border_color(Color::magenta).padding_top(1).padding_bottom(1).width(TBL_W);
        printCenteredBlankLine(); printCentered(note);
        printDashMenu("Portal Options",{{"[1]","Login Account"},{"[2]","Register Account"}},Color::magenta);
        cin>>ch; if(cin.fail()){clearInput();continue;}
        if(ch==1){
            string role=chooseRole("LOGIN",Color::magenta);
            if(!role.empty()){string user; if(doLogin(role,user)) return{role,user}; else pressEnter();}
        } else if(ch==2){
            string role=chooseRole("REGISTER",Color::magenta);
            if(!role.empty()){registerUser(role); pressEnter();}
        }
    } while(true);
}

static bool doLogin(const string& role,string& outUser) {
    clearScreen();
    const char* tc=(role=="librarian")?SP_MAG:SP_CYAN;
    printFigletTitle(role=="librarian"?"LIBRARIAN":"STUDENT",tc);
    printFigletTitle("LOGIN",tc);
    bool ok=loginUser(role,outUser);
    if(ok){
        logSession(outUser,role,"LOGIN");   // ← log login
        clearScreen();
        Table w; w.add_row({"  Welcome, "+outUser+"!  System ready.  "});
        w[0].format().font_align(FontAlign::center).font_style({FontStyle::bold})
            .font_color(Color::green).border_color(Color::green).padding_top(1).padding_bottom(1).width(TBL_W);
        printCenteredBlankLine(); printCentered(w);
        this_thread::sleep_for(chrono::milliseconds(600));
    } else {
        Table d; d.add_row({"  [!!]  Access Denied  --  Invalid credentials  "});
        d[0].format().font_align(FontAlign::center).font_style({FontStyle::bold})
            .font_color(Color::red).border_color(Color::red).padding_top(1).padding_bottom(1).width(TBL_W);
        printCenteredBlankLine(); printCentered(d);
        this_thread::sleep_for(chrono::milliseconds(1400));
    }
    return ok;
}

static void runMainMenu() {
    int ch; string loggedUser;
    do {
        syncFromDisk(); clearScreen();
        printFigletTitle("LIBRARY",SP_MAG); printFigletTitle("SYSTEM",SP_MAG);
        Table st; st.add_row({"  v  System Ready  --  Books: "+to_string(books.size())+"  v  "});
        st[0].format().font_align(FontAlign::center).font_color(Color::green)
            .border_color(Color::green).font_style({FontStyle::bold}).width(TBL_W);
        printCenteredBlankLine(); printCentered(st);
        printDashMenu("Main Menu",{{"[ADM]","Librarian Login"},{"[STU]","Student Login"},{"[OUT]","Exit"}},Color::magenta);
        cin>>ch; if(cin.fail()){clearInput();continue;}
        switch(ch){
            case 1: if(doLogin("librarian",loggedUser)) librarianMenu(loggedUser); break;
            case 2: if(doLogin("student",loggedUser))   studentMenu(loggedUser);   break;
            case 3:
                saveBooks(books); clearScreen(); printFigletTitle("GOODBYE",SP_CYAN);
                {Table b; b.add_row({"  Thank you for using the Library System!  "});
                 b[0].format().font_align(FontAlign::center).font_style({FontStyle::bold})
                     .font_color(Color::cyan).border_color(Color::cyan).padding_top(1).padding_bottom(1).width(TBL_W);
                 printCenteredBlankLine(); printCentered(b); printCenteredBlankLine();}
                this_thread::sleep_for(chrono::milliseconds(1000)); break;
            default: break;
        }
    } while(ch!=3);
}

int main() {
#ifdef _WIN32
    _putenv("LC_ALL=C"); _putenv("LANG=C"); _putenv("LANGUAGE=C");
#endif
    setlocale(LC_ALL,"C");
    try { std::locale::global(std::locale::classic()); } catch(...) {}
    initConsole();
    loadBooks(books); loadBorrows(borrows);
    showStartupSplash();
    auto [loggedRole,loggedUser]=runAccessPortal();
    if(loggedRole=="librarian") librarianMenu(loggedUser);
    else                        studentMenu(loggedUser);
    runMainMenu();
    return 0;
}