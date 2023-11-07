#pragma once

#include <string>

struct User {
    User();
    User(const wchar_t* username, const wchar_t* password, bool isBlocked, bool isRestrictionEnabled);

    friend std::ofstream& operator<<(std::ofstream& ofs, const User& user);
    friend std::ifstream& operator>>(std::ifstream& ifs, User& user);

    std::wstring username;
    std::wstring password;
    bool isBlocked;
    bool isRestrictionEnabled;
};
