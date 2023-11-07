#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <string>
#include <cstdlib>
#include <fstream>
#include <vector>

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
        user.username.resize(usernameLength + 1);
        user.password.resize(passwordLength + 1);
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
};

struct LoginResult {
    std::wstring username;
    std::wstring password;
    std::wstring handshake;
    LoginStatus result;

    LoginResult(const std::wstring& username, const std::wstring& password, const std::wstring& handshake, LoginStatus result)
    : username(username), password(password), handshake(handshake), result(result) { }
};

constexpr const wchar_t* databaseFile = L"users.dat";

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
        EndDialog(hwnd, 0);
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
            username.resize(loginLength + 1);
            password.resize(passwordLength + 1);
            handshake.resize(handshakeLength + 1);
            GetDlgItemTextW(hwnd, IDC_EDIT1, &username[0], loginLength + 1);
            GetDlgItemTextW(hwnd, IDC_EDIT2, &password[0], passwordLength + 1);
            GetDlgItemTextW(hwnd, IDC_EDIT3, &handshake[0], handshakeLength + 1);
            // If user doesn't exist - show warning
            LoginInput* input = (LoginInput*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
            bool userExist = false;
            for (const User& user : input->database.users) {
                if (username == user.username) {
                    userExist = true;
                    break;
                }
            }

            if (!userExist) {
                MessageBoxW(hwnd, L"User with such name doesn't exist", L"Warning", MB_OK | MB_ICONERROR);
                break;
            }



            // Save data and return it
            EndDialog(hwnd, (INT_PTR)new LoginResult(username, password, handshake, LoginStatus::LOGIN));
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
    std::ifstream file(databaseFile, std::ios::binary);
    Database database;
    if (!file.good()) {
        database.users.emplace_back(L"ADMIN", L"", false, false);
        std::ofstream newFile(databaseFile, std::ios::binary);
        newFile << database;
        newFile.close();
        file.open(databaseFile, std::ios::binary);
    }

    file >> database;
    Console::GetInstance()->WPrintF(L"Username: %s\nPassword: %s\nIsBlocked: %d\nisRestrictionEnabled: %d\n",
        database.users[0].username.c_str(), database.users[0].password.c_str(), database.users[0].isBlocked ? 1 : 0, database.users[0].isRestrictionEnabled ? 1 : 0);

    LoginInput loginParams = { database, rand() % 1000 };
    struct LoginResult* result = (struct LoginResult*)DialogBoxParamW(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), nullptr, LoginProc, (LPARAM)&loginParams);
    if (result == nullptr) {
        Console::GetInstance()->WPrintF(L"NO DATA\n");
        Console::GetInstance()->Pause();
        return 0;
    }

    Console::GetInstance()->WPrintF(L"Username: %s\nPassword: %s\nHandshake: %s", result->username, result->password, result->handshake);
    delete result;

    Console::GetInstance()->Pause();
	return 0;
}
