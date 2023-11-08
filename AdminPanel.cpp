#include "Database.h"
#include "AdminPanel.h"
#include "UserPanel.h"
#include "Constants.h"
#include "resource.h"

LRESULT CALLBACK UserProfileProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
    {
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, lParam);
        HWND hUserName = GetDlgItem(hwnd, IDC_USER_USERNAME);
        HWND hBlocked = GetDlgItem(hwnd, IDC_CHECK_BLOCKED);
        HWND hRestrictions = GetDlgItem(hwnd, IDC_CHECK_RESTRICTION);
        std::wstring text = L"User: " + ((const User*)lParam)->username;
        SetWindowTextW(hUserName, text.c_str());
        if (((const User*)lParam)->isBlocked) {
            SendMessageW(hBlocked, BM_SETCHECK, BST_CHECKED, 0);
        }
        
        if (((const User*)lParam)->isRestrictionEnabled) {
            SendMessageW(hRestrictions, BM_SETCHECK, BST_CHECKED, 0);
        }

        break;
    }
    case WM_CLOSE:
        EndDialog(hwnd, 0);
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_CHECK_BLOCKED && HIWORD(wParam) == BN_CLICKED) {
            HWND hBlocked = GetDlgItem(hwnd, IDC_CHECK_BLOCKED);
            ((User*)GetWindowLongPtrW(hwnd, GWLP_USERDATA))->isBlocked = SendMessageW(hBlocked, BM_GETCHECK, 0, 0) == BST_CHECKED;

            break;
        }

        if (LOWORD(wParam) == IDC_CHECK_RESTRICTION && HIWORD(wParam) == BN_CLICKED) {
            HWND hRestrictions = GetDlgItem(hwnd, IDC_CHECK_RESTRICTION);
            ((User*)GetWindowLongPtrW(hwnd, GWLP_USERDATA))->isRestrictionEnabled = SendMessageW(hRestrictions, BM_GETCHECK, 0, 0) == BST_CHECKED;

            break;
        }

        break;
    default:
        return FALSE;
    }

    return TRUE;
}

LRESULT CALLBACK AdminPanelProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
    {
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, lParam);
        HWND hUserName = GetDlgItem(hwnd, IDC_USER_USERNAME);
        std::wstring text = L"User: " + ((const UserPanelInput*)lParam)->user->username;
        SetWindowTextW(hUserName, text.c_str());

        HWND hListBox = GetDlgItem(hwnd, IDC_LIST_USERS);
        for (const User& user : ((const UserPanelInput*)lParam)->database.users) {
            if (user.username == ((const UserPanelInput*)lParam)->user->username) {
                continue;
            }
        }

        break;
    }
    case WM_CLOSE:
        EndDialog(hwnd, 0);
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_MENU_ABOUTPROGRAM) {
            MessageBoxW(hwnd, kAboutMessage, L"About Program", MB_ICONINFORMATION | MB_OK);
            break;
        }
        else if (LOWORD(wParam) == ID_USER_CHANGEPASS && HIWORD(wParam) == BN_CLICKED) {
            // Change password. Update database
            const UserPanelInput* input = (const UserPanelInput*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
            bool status = DialogBoxParamW(GetModuleHandleW(nullptr), MAKEINTRESOURCE(IDD_CHANGEPASSWORD), hwnd, ChangePasswordProc, (LPARAM)input->user);
            if (status) {
                input->database.Save();
            }

            break;
        }

        // TODO
        // Add user (Dialog username)
        // Add handshake handling
        else if (LOWORD(wParam) == IDC_LIST_USERS && HIWORD(wParam) == LBN_DBLCLK) {
            HWND hListBox = GetDlgItem(hwnd, IDC_LIST_USERS);
            LRESULT selectedIndex = SendMessageW(hListBox, LB_GETCURSEL, 0, 0);
            if (selectedIndex == LB_ERR) {
                break;
            }

            std::wstring selectedUsername;
            selectedUsername.resize(SendMessageW(hListBox, LB_GETTEXTLEN, selectedIndex, 0));
            SendMessageW(hListBox, LB_GETTEXT, selectedIndex, (LPARAM)&selectedUsername[0]);

            Database& database = ((UserPanelInput*)GetWindowLongPtrW(hwnd, GWLP_USERDATA))->database;
            User* user = nullptr;
            for (User& curr : database.users) {
                if (curr.username == selectedUsername) {
                    user = &curr;
                    break;
                }
            }

            DialogBoxParamW(GetModuleHandleW(nullptr), MAKEINTRESOURCE(IDD_DIALOG_USER_PROFILE), nullptr, UserProfileProc, (LPARAM)user);
            database.Save();
        }

        break;
    default:
        return FALSE;
    }

    return TRUE;
}
