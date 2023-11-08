#pragma once

#include <string>

constexpr const wchar_t* kDatabaseFile = L"users.dat";
constexpr const wchar_t* kAdminUsername = L"ADMIN";
constexpr const wchar_t* kAboutMessage = L"Made by Kostin A.S. student of CS-920d group.\n\nIndividual Task:\nPassword type: Handshake\nPassword restrictions: Password should have latin, cyrillic symbols and digits";
constexpr const int kAttempts = 3;

bool IsPasswordValid(const std::wstring& password);
