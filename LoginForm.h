#pragma once

#include <Windows.h>
#include "Database.h"

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
    LoginResult(User* user, LoginStatus result);

    User* user;
    LoginStatus result;
};

LRESULT CALLBACK RepeatProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LoginProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
