#include <string>

#include "Constants.h"

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
