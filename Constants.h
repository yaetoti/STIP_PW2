#pragma once

#include <string>

constexpr const wchar_t* kDatabaseFile = L"users.dat";
constexpr const wchar_t* kAdminUsername = L"ADMIN";
constexpr const int kAttempts = 3;

bool IsPasswordValid(const std::wstring& password);
