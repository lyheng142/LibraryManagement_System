#ifndef TERMINAL_LAYOUT_HPP
#define TERMINAL_LAYOUT_HPP

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>

#ifdef _WIN32
  #include <windows.h>
#else
  #include <sys/ioctl.h>
  #include <unistd.h>
#endif

#include <tabulate/table.hpp>

inline std::size_t getTerminalWidth() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return static_cast<std::size_t>(csbi.srWindow.Right - csbi.srWindow.Left + 1);
    }
#else
    winsize w{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) {
        return static_cast<std::size_t>(w.ws_col);
    }
#endif
    if (const char* env = std::getenv("COLUMNS")) {
        try {
            int v = std::stoi(env);
            if (v > 0) return static_cast<std::size_t>(v);
        } catch (...) {}
    }
    return 100;
}

inline std::string stripAnsi(const std::string& s) {
    std::string out;
    out.reserve(s.size());

    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\033') {
            ++i;
            if (i < s.size() && s[i] == '[') {
                ++i;
                while (i < s.size()) {
                    const unsigned char ch = static_cast<unsigned char>(s[i]);
                    if (ch >= 0x40 && ch <= 0x7E) break;
                    ++i;
                }
            }
            continue;
        }
        out.push_back(s[i]);
    }
    return out;
}

inline std::size_t visibleWidth(const std::string& s) {
    std::size_t count = 0;
    const std::string clean = stripAnsi(s);
    for (unsigned char ch : clean) {
        if ((ch & 0xC0) != 0x80) ++count;
    }
    return count;
}

inline std::size_t maxVisibleLineWidth(const std::string& block) {
    std::istringstream iss(block);
    std::string line;
    std::size_t best = 0;
    while (std::getline(iss, line)) {
        best = std::max(best, visibleWidth(line));
    }
    return best;
}

inline std::size_t centeredLeftPad(std::size_t contentWidth) {
    const std::size_t terminalWidth = getTerminalWidth();
    if (terminalWidth <= contentWidth) return 0;
    return (terminalWidth - contentWidth) / 2;
}

inline std::string centerBlock(const std::string& block) {
    std::istringstream iss(block);
    std::ostringstream oss;
    std::string line;
    bool first = true;
    const std::size_t pad = centeredLeftPad(maxVisibleLineWidth(block));

    while (std::getline(iss, line)) {
        if (!first) oss << '\n';
        first = false;
        oss << std::string(pad, ' ') << line;
    }

    if (!block.empty() && block.back() == '\n') {
        oss << '\n';
    }
    return oss.str();
}

inline void printCenteredText(const std::string& block, bool newlineAfter = false) {
    std::cout << centerBlock(block);
    if (newlineAfter) std::cout << '\n';
}

inline std::string tableToString(const tabulate::Table& t) {
    std::ostringstream oss;
    oss << t;
    return oss.str();
}

class IndentingStreamBuf : public std::streambuf {
public:
    IndentingStreamBuf(std::streambuf* dest, std::string prefix)
        : dest_(dest), prefix_(std::move(prefix)) {}

protected:
    int_type overflow(int_type ch) override {
        if (traits_type::eq_int_type(ch, traits_type::eof())) {
            return traits_type::not_eof(ch);
        }

        if (at_line_start_ && ch != '\n') {
            if (dest_->sputn(prefix_.data(), static_cast<std::streamsize>(prefix_.size())) !=
                static_cast<std::streamsize>(prefix_.size())) {
                return traits_type::eof();
            }
        }

        at_line_start_ = (ch == '\n');
        return dest_->sputc(static_cast<char>(ch));
    }

    std::streamsize xsputn(const char* s, std::streamsize count) override {
        std::streamsize written = 0;
        for (; written < count; ++written) {
            if (traits_type::eq_int_type(overflow(traits_type::to_int_type(s[written])), traits_type::eof())) {
                break;
            }
        }
        return written;
    }

    int sync() override {
        return dest_->pubsync();
    }

private:
    std::streambuf* dest_;
    std::string prefix_;
    bool at_line_start_ = true;
};

class ScopedStreamRedirect {
public:
    ScopedStreamRedirect(std::ostream& stream, std::streambuf* new_buffer)
        : stream_(stream), old_buffer_(stream.rdbuf(new_buffer)) {}

    ~ScopedStreamRedirect() {
        stream_.rdbuf(old_buffer_);
    }

    ScopedStreamRedirect(const ScopedStreamRedirect&) = delete;
    ScopedStreamRedirect& operator=(const ScopedStreamRedirect&) = delete;

private:
    std::ostream& stream_;
    std::streambuf* old_buffer_;
};

inline void printCentered(const tabulate::Table& t, bool newlineAfter = true) {
    const std::size_t pad = centeredLeftPad(maxVisibleLineWidth(tableToString(t)));
    if (pad == 0) {
        std::cout << t;
    } else {
        IndentingStreamBuf indenting_buf(std::cout.rdbuf(), std::string(pad, ' '));
        ScopedStreamRedirect redirect(std::cout, &indenting_buf);
        std::cout << t;
    }
    if (newlineAfter) std::cout << '\n';
}

inline void printCenteredBlankLine() {
    std::cout << '\n';
}

#endif
