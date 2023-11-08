#include "UserPanel.h"
#include "Constants.h"
#include "resource.h"

LRESULT CALLBACK ChangePasswordProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, lParam);
        break;
    case WM_CLOSE:
        EndDialog(hwnd, false);
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_CHANGEPASS_OK && HIWORD(wParam) == BN_CLICKED) {
            HWND hPassword = GetDlgItem(hwnd, IDC_PASSWORD);
            HWND hNewPassword = GetDlgItem(hwnd, IDC_NEWPASSWORD);
            HWND hRepeatNewPassword = GetDlgItem(hwnd, IDC_REPEATNEWPASSWORD);
            int passwordLength = GetWindowTextLengthW(hPassword);
            int newPasswordLength = GetWindowTextLengthW(hNewPassword);
            int repeatNewPasswordLength = GetWindowTextLengthW(hRepeatNewPassword);
            if (passwordLength == 0 || newPasswordLength == 0 || repeatNewPasswordLength == 0) {
                break;
            }

            std::wstring password;
            std::wstring newPassword;
            std::wstring repeatNewPassword;
            password.resize(passwordLength);
            newPassword.resize(newPasswordLength);
            repeatNewPassword.resize(repeatNewPasswordLength);
            GetDlgItemTextW(hwnd, IDC_PASSWORD, &password[0], passwordLength + 1);
            GetDlgItemTextW(hwnd, IDC_NEWPASSWORD, &newPassword[0], newPasswordLength + 1);
            GetDlgItemTextW(hwnd, IDC_REPEATNEWPASSWORD, &repeatNewPassword[0], repeatNewPasswordLength + 1);

            User* user = (User*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
            if (password != user->password) {
                MessageBoxW(hwnd, L"Wrong password!", L"Warning", MB_OK | MB_ICONERROR);
                break;
            }

            if (password == newPassword) {
                MessageBoxW(hwnd, L"Passwords shouldn't be the same!", L"Warning", MB_OK | MB_ICONERROR);
                break;
            }

            if (user->isRestrictionEnabled && !IsPasswordValid(newPassword)) {
                MessageBoxW(hwnd, L"Password must contain latin, cyrillic characters and numbers!", L"Warning", MB_OK | MB_ICONERROR);
                break;
            }

            if (newPassword != repeatNewPassword) {
                MessageBoxW(hwnd, L"Passwords don't match!", L"Warning", MB_OK | MB_ICONERROR);
                break;
            }

            user->password = newPassword;
            EndDialog(hwnd, true);
        }

        break;
    default:
        return FALSE;
    }

    return TRUE;
}

LRESULT CALLBACK UserPanelProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
    {
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, lParam);
        HWND hUserName = GetDlgItem(hwnd, IDC_USER_USERNAME);
        std::wstring text = L"User: " + ((const UserPanelInput*)lParam)->user->username;
        SetWindowTextW(hUserName, text.c_str());
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

        break;
    default:
        return FALSE;
    }

    return TRUE;
}