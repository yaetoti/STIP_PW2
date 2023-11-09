#include <fstream>

#include "Database.h"
#include "Constants.h"

Database::Database(const wchar_t* filename)
    : filename(filename) {
    std::ifstream file(filename, std::ios::binary);
    if (file.is_open()) {
        file >> *this;
    }
    else {
        users.emplace_back(std::make_unique<User>(kAdminUsername, L"", false, false));
    }
}

void Database::Save() const {
    std::ofstream newFile(filename, std::ios::binary | std::ios::trunc);
    newFile << *this;
}

std::ofstream& operator<<(std::ofstream& ofs, const Database& database) {
    // Format: [size][user0]...
    size_t size = database.users.size();
    ofs.write((const char*)&size, sizeof(size));
    for (size_t i = 0; i < size; ++i) {
        ofs << *database.users[i];
    }

    return ofs;
}

std::ifstream& operator>>(std::ifstream& ifs, Database& database) {
    size_t size;
    ifs.read((char*)&size, sizeof(size));
    database.users.resize(size);
    User* user;
    for (size_t i = 0; i < size; ++i) {
        user = new User();
        ifs >> *user;
        database.users[i].reset(user);
    }

    return ifs;
}
