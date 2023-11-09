#pragma once

#include <vector>
#include <string>
#include <memory>

#include "User.h"

struct Database {
    Database(const wchar_t* filename);

    void Save() const;

    friend std::ofstream& operator<<(std::ofstream& ofs, const Database& database);
    friend std::ifstream& operator>>(std::ifstream& ifs, Database& database);

    std::vector<std::unique_ptr<User>> users;
    const wchar_t* filename;
};
