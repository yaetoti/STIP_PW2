#include "Database.h"
#include "AdminPanel.h"
#include "UserPanel.h"
#include "Constants.h"
#include "resource.h"

LRESULT CALLBACK AdminPanelProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
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
