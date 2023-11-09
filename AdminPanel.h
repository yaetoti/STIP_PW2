#pragma once

#include <Windows.h>

LRESULT CALLBACK AddUserProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK UserProfileProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK AdminPanelProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
