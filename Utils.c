#include "MemoryDump.h"

DWORD ParsePid(const wchar_t* str) {
    if (!str) return 0;
    wchar_t* endptr = NULL;
    errno = 0;
    unsigned long long val = wcstoull(str, &endptr, 10);
    if (errno != 0 || *endptr != L'\0' || val == 0 || val > MAXDWORD) {
        return 0;
    }
    return (DWORD) val;
}

ULONG_PTR ParseAddress(const wchar_t* str) {
    if (!str) return 0;
    wchar_t* endptr = NULL;
    errno = 0;
    unsigned long long val = wcstoull(str, &endptr, 16);
    if (errno != 0 || val > MAXULONG_PTR) {
        return 0;
    }
    if (*endptr == L'\0') {
        return (ULONG_PTR) val;
    }
    val = wcstoull(str, &endptr, 10);
    if (errno != 0 || *endptr != L'\0' || val > MAXULONG_PTR) {
        return 0;
    }
    return (ULONG_PTR) val;
}

SIZE_T ParseSize(const wchar_t* str) {
    if (!str) return 0;
    wchar_t* endptr = NULL;
    errno = 0;
    unsigned long long val = wcstoull(str, &endptr, 0);
    if (errno != 0 || val > MAXSIZE_T) {
        return 0;
    }
    return (SIZE_T) val;
}

DWORD GetPidByName(const wchar_t* name) {
    PROCESSENTRY32W pe32 = { 0 };
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    DWORD foundPid = 0;
    DWORD count = 0;

    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            if (_wcsicmp(pe32.szExeFile, name) == 0) {
                foundPid = pe32.th32ProcessID;
                count++;
            }
        } while (Process32NextW(hSnapshot, &pe32) && count < 2);
    }
    CloseHandle(hSnapshot);

    if (count == 0) {
        return 0;
    } else if (count > 1) {
        return (DWORD) -1;
    }
    return foundPid;
}
