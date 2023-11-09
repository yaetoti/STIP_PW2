#include <fstream>

#include "User.h"

User::User()
    : isBlocked(false), isRestrictionEnabled(false) {}

User::User(const wchar_t* username, const wchar_t* password, bool isBlocked, bool isRestrictionEnabled)
    : username(username), password(password), isBlocked(isBlocked), isRestrictionEnabled(isRestrictionEnabled) {
}

User::User(User&& other) noexcept
    : username(std::move(other.username)), password(std::move(other.password)), isBlocked(other.isBlocked), isRestrictionEnabled(other.isRestrictionEnabled) {
    other.isBlocked = false;
    other.isRestrictionEnabled = false;
}

std::ofstream& operator<<(std::ofstream& ofs, const User& user) {
    // Format: [usernameLength][passwordLength][username][password][isBlocked][isRestrictionEnabled]
    size_t usernameLength = user.username.length();
    size_t passwordLength = user.password.length();
    ofs.write((const char*)&usernameLength, sizeof(usernameLength));
    ofs.write((const char*)&passwordLength, sizeof(passwordLength));
    ofs.write((const char*)user.username.c_str(), sizeof(wchar_t) * usernameLength);
    ofs.write((const char*)user.password.c_str(), sizeof(wchar_t) * passwordLength);
    ofs.write((const char*)&user.isBlocked, sizeof(bool));
    ofs.write((const char*)&user.isRestrictionEnabled, sizeof(bool));
    return ofs;
}

std::ifstream& operator>>(std::ifstream& ifs, User& user) {
    size_t usernameLength;
    size_t passwordLength;
    ifs.read((char*)&usernameLength, sizeof(usernameLength));
    ifs.read((char*)&passwordLength, sizeof(passwordLength));
    user.username.resize(usernameLength);
    user.password.resize(passwordLength);
    user.username[usernameLength] = user.password[passwordLength] = L'\0';
    ifs.read((char*)&user.username[0], sizeof(wchar_t) * usernameLength);
    ifs.read((char*)&user.password[0], sizeof(wchar_t) * passwordLength);
    ifs.read((char*)&user.isBlocked, sizeof(bool));
    ifs.read((char*)&user.isRestrictionEnabled, sizeof(bool));
    return ifs;
}
