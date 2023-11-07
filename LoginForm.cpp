#include "LoginForm.h"
#include "Constants.h"
#include "resource.h"

LoginResult::LoginResult(User* user, LoginStatus result)
    : user(user), result(result) { }

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
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, lParam);
        // Update handshake text
        HWND hHandshake = GetDlgItem(hwnd, IDC_HANDSHAKE);
        std::wstring text = L"Handshake: " + std::to_wstring(((const LoginInput*)lParam)->handshake);
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

            if (user->isBlocked) {
                MessageBoxW(hwnd, L"Account is blocked!", L"Warning", MB_OK | MB_ICONERROR);
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
