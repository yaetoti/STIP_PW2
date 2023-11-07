#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <cstdlib>
#include <memory>

#include "resource.h"
#include "Console.h"
#include "Constants.h"
#include "Database.h"
#include "LoginForm.h"

#pragma comment(lib, "ConsoleLib")

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    srand(GetTickCount());
    Database database(kDatabaseFile);
    User* user;
    // Show login form
    {
        LoginInput loginParams = { database, rand() % 1000, kAttempts };
        std::unique_ptr<LoginResult> result((LoginResult*)DialogBoxParamW(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), nullptr, LoginProc, (LPARAM)&loginParams));
        if (result->result == LoginStatus::CANCEL) {
            Console::GetInstance()->WPrintF(L"NO DATA\n");
            Console::GetInstance()->Pause();
            return 0;
        }

        // Update database if needed
        if (result->result == LoginStatus::UPDATE) {
            database.Save();
        }

        user = result->user;
    }

    Console::GetInstance()->WPrintF(L"Username: %s\nPassword: %s\n", user->username.c_str(), user->password.c_str());

    // Show main form

    Console::GetInstance()->Pause();
	return 0;
}
