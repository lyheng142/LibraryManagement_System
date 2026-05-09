╔══════════════════════════════════════════════════════════════╗
║          LIBRARY MANAGEMENT SYSTEM                          ║
║          Works on: Windows 10/11  and  Linux                ║
╚══════════════════════════════════════════════════════════════╝

━━━  FOR WINDOWS USERS  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  STEP 1 — Install CMake (if not installed)
    Download: https://cmake.org/download/
    IMPORTANT: tick "Add CMake to system PATH" during install

  STEP 2 — Install MinGW compiler (if not installed)
    Download: https://github.com/niXman/mingw-builds-binaries/releases
    Pick file named: x86_64-...-release-win32-seh-ucrt-...7z
    Extract to C:\mingw64
    Add C:\mingw64\bin to your PATH environment variable

  STEP 3 — Build the project
    Double-click: build_windows.bat
    Wait for it to finish (takes 1-2 minutes first time)

  STEP 4 — Run the program
    Double-click: run.bat  (created automatically after build)

━━━  FOR LINUX USERS  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  STEP 1 — Install tools (if not installed)
    sudo apt install cmake g++

  STEP 2 — Build and run
    chmod +x build_linux.sh
    ./build_linux.sh
    ./build/library

━━━  DEFAULT ACCOUNTS  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  Librarian : username = admin     password = admin123
  Student   : username = student   password = 1234

━━━  DATA FILES  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  data/books.xlsx   — book records
  data/borrow.xlsx  — borrow history
  data/users.xlsx   — user accounts

  Open these in Microsoft Excel or Google Sheets anytime!

━━━  WINDOWS DISPLAY TIP  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  Use Windows Terminal for best display (colors + Unicode)
  Download free from Microsoft Store: search "Windows Terminal"

╔══════════════════════════════════════════════════════════════╗
║  IMPORTANT: Always run from the project ROOT folder,        ║
║  NOT from inside the build/ folder!                         ║
╚══════════════════════════════════════════════════════════════╝
