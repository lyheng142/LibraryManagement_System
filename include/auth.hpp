#ifndef AUTH_HPP
#define AUTH_HPP

#include <string>

// roles: "librarian"  or  "student"
bool loginUser(const std::string& requiredRole, std::string& loggedInUsername);
bool registerUser(const std::string& role);

#endif
