#ifndef AUTH_HPP
#define AUTH_HPP

#include <string>

bool loginUser    (const std::string& requiredRole, std::string& loggedInUsername);
bool registerUser (const std::string& role);
void manageUsers  ();
bool changePassword(const std::string& username, const std::string& role);
void logSession    (const std::string& username, const std::string& role, const std::string& action);
void viewSessionLog();

#endif