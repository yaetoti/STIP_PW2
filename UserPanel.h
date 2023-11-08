#pragma once

#include <Windows.h>

#include "Database.h"

struct UserPanelInput {
    Database& database;
    User* user;
};

LRESULT CALLBACK ChangePasswordProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK UserPanelProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
