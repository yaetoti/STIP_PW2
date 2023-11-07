#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <string>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <cwctype>

#include "resource.h"
#include "Console.h"

#pragma comment(lib, "ConsoleLib")

struct User {
    std::wstring username;
    std::wstring password;
    bool isBlocked;
    bool isRestrictionEnabled;

    User()
    : isBlocked(false), isRestrictionEnabled (false) {}

    User(const wchar_t* username, const wchar_t* password, bool isBlocked, bool isRestrictionEnabled)
        : username(username), password(password), isBlocked(isBlocked), isRestrictionEnabled(isRestrictionEnabled) {
    }

    friend std::ofstream& operator<<(std::ofstream& ofs, const User& user) {
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

    friend std::ifstream& operator>>(std::ifstream& ifs, User& user) {
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
};

struct Database {
    std::vector<User> users;

    friend std::ofstream& operator<<(std::ofstream& ofs, const Database& database) {
        // Format: [size][user0]...
        size_t size = database.users.size();
        ofs.write((const char*)&size, sizeof(size));
        for (size_t i = 0; i < size; ++i) {
            ofs << database.users[i];
        }

        return ofs;
    }

    friend std::ifstream& operator>>(std::ifstream& ifs, Database& database) {
        size_t size;
        ifs.read((char*)&size, sizeof(size));
        database.users.resize(size);
        for (size_t i = 0; i < size; ++i) {
            ifs >> database.users[i];
        }

        return ifs;
    }
};

enum class LoginStatus : INT_PTR {
    LOGIN, // User authorized
    UPDATE, // User set password and authorized
    CANCEL // 
};

struct LoginInput {
    Database& database;
    int handshake;
    int attempts;
};

struct LoginResult {
    User* user;
    LoginStatus result;

    LoginResult(User* user, LoginStatus result)
    : user(user), result(result) { }
};

constexpr const wchar_t* kDatabaseFile = L"users.dat";
constexpr const wchar_t* kAdminUsername = L"ADMIN";
constexpr const int kAttempts = 3;

bool IsPasswordValid(const std::wstring& password) {
    bool hasLatin = false;
    bool hasCyrillic = false;
    bool hasNumbers = false;
    for (wchar_t ch : password) {
        if (iswalpha(ch)) {
            if (iswascii(ch)) {
                hasLatin = true;
            }
            else if (ch >= 0x0400 && ch <= 0x04FF) {
                hasCyrillic = true;
            }
            else {
                return false;
            }
        }
        else if (iswdigit(ch)) {
            hasNumbers = true;
        }
        else {
            return false;
        }
    }

    return hasLatin && hasCyrillic && hasNumbers;
}

LRESULT CALLBACK RepeatProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        // Save input data
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, lParam);
        break;
    case WM_CLOSE:
        EndDialog(hwnd, false);
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK && HIWORD(wParam) == BN_CLICKED) {
            // Get input and check matching
            const std::wstring* original = (const std::wstring*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
            HWND hPassword = GetDlgItem(hwnd, IDC_EDIT_REPEAT);
            int passwordLength = GetWindowTextLengthW(hPassword);
            if (passwordLength != original->length()) {
                MessageBoxW(hwnd, L"Passwords don't match!", L"Warning", MB_OK | MB_ICONERROR);
                break;
            }

            std::wstring password;
            password.resize(passwordLength);
            GetDlgItemTextW(hwnd, IDC_EDIT_REPEAT, &password[0], passwordLength + 1);
            if (password != *original) {
                MessageBoxW(hwnd, L"Passwords don't match!", L"Warning", MB_OK | MB_ICONERROR);
                break;
            }

            EndDialog(hwnd, true);
        }

        break;
    default:
        return FALSE;
    }

    return TRUE;
}

LRESULT CALLBACK LoginProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
    {
        // Save input data
        LoginInput* input = (LoginInput*)lParam;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, lParam);
        // Update handshake text
        HWND hHandshake = GetDlgItem(hwnd, IDC_HANDSHAKE);
        std::wstring text = L"Handshake: " + std::to_wstring(input->handshake);
        SetWindowTextW(hHandshake, text.c_str());
        break;
    }
    case WM_CLOSE:
        EndDialog(hwnd, (INT_PTR)new LoginResult(nullptr, LoginStatus::CANCEL));
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK && HIWORD(wParam) == BN_CLICKED) {
            // Read user input
            HWND hLogin = GetDlgItem(hwnd, IDC_EDIT1);
            HWND hPassword = GetDlgItem(hwnd, IDC_EDIT2);
            HWND hHandshake = GetDlgItem(hwnd, IDC_EDIT3);
            int loginLength = GetWindowTextLengthW(hLogin);
            int passwordLength = GetWindowTextLengthW(hPassword);
            int handshakeLength = GetWindowTextLengthW(hHandshake);
            if (loginLength == 0 || passwordLength == 0 || handshakeLength == 0) {
                break;
            }
            std::wstring username;
            std::wstring password;
            std::wstring handshake;
            username.resize(loginLength);
            password.resize(passwordLength);
            handshake.resize(handshakeLength);
            GetDlgItemTextW(hwnd, IDC_EDIT1, &username[0], loginLength + 1);
            GetDlgItemTextW(hwnd, IDC_EDIT2, &password[0], passwordLength + 1);
            GetDlgItemTextW(hwnd, IDC_EDIT3, &handshake[0], handshakeLength + 1);
            // If user doesn't exist - show warning
            LoginInput* input = (LoginInput*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
            User* user = nullptr;
            for (User& currentUser : input->database.users) {
                if (username == currentUser.username) {
                    user = &currentUser;
                    break;
                }
            }

            if (user == nullptr) {
                MessageBoxW(hwnd, L"User with such name doesn't exist!", L"Warning", MB_OK | MB_ICONERROR);
                break;
            }

            // If user wasn't registered
            if (user->password.empty()) {
                // Validate if set restriction
                if (user->isRestrictionEnabled && !IsPasswordValid(password)) {
                    MessageBoxW(hwnd, L"Password must contain latin, cyrillic characters and numbers!", L"Warning", MB_OK | MB_ICONERROR);
                    break;
                }

                // Ask to repeat password
                bool match = DialogBoxParamW(GetModuleHandleW(nullptr), MAKEINTRESOURCE(IDD_DIALOG_REPEAT), hwnd, RepeatProc, (LPARAM)&password);
                if (!match) {
                    break;
                }

                // Match - change pass and return
                user->password = password;
                EndDialog(hwnd, (INT_PTR)new LoginResult(user, LoginStatus::UPDATE));
                break;
            }

            // User was registered. Check password. 3 times
            if (user->password != password) {
                MessageBoxW(hwnd, L"Wrong password!", L"Warning", MB_OK | MB_ICONERROR);
                --input->attempts;
                // No attempts left. Exit
                if (input->attempts <= 0) {
                    EndDialog(hwnd, (INT_PTR)new LoginResult(nullptr, LoginStatus::CANCEL));
                }

                break;
            }

            // Password match. Authorize user
            EndDialog(hwnd, (INT_PTR)new LoginResult(user, LoginStatus::LOGIN));
        }

        break;
    default:
        return FALSE;
    }

    return TRUE;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    srand(GetTickCount());
    // Open database. If doesn't exist - create
    Database database;
    {
        std::ifstream file(kDatabaseFile, std::ios::binary);
        if (!file.good()) {
            database.users.emplace_back(kAdminUsername, L"", false, false);
            std::ofstream newFile(kDatabaseFile, std::ios::binary);
            newFile << database;
            newFile.close();
            file.open(kDatabaseFile, std::ios::binary);
        }

        file >> database;
        file.close();
    }

    Console::GetInstance()->WPrintF(L"Username: %s\nPassword: %s\nIsBlocked: %d\nisRestrictionEnabled: %d\n",
        database.users[0].username.c_str(), database.users[0].password.c_str(), database.users[0].isBlocked ? 1 : 0, database.users[0].isRestrictionEnabled ? 1 : 0);

    LoginInput loginParams = { database, rand() % 1000, kAttempts };
    LoginResult* result = (LoginResult*)DialogBoxParamW(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), nullptr, LoginProc, (LPARAM)&loginParams);
    if (result->result == LoginStatus::CANCEL) {
        Console::GetInstance()->WPrintF(L"NO DATA\n");
        Console::GetInstance()->Pause();
        return 0;
    }

    if (result->result == LoginStatus::UPDATE) {
        std::ofstream newFile(kDatabaseFile, std::ios::binary | std::ios::trunc);
        newFile << database;
        newFile.close();
    }

    Console::GetInstance()->WPrintF(L"Username: %s\nPassword: %s\n", result->user->username.c_str(), result->user->password.c_str());
    delete result;

    Console::GetInstance()->Pause();
	return 0;
}
