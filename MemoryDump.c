#include "MemoryDump.h"

#define OPT_PID 'p'
#define OPT_NAME 'n'
#define OPT_ADDR_START 'a'
#define OPT_ADDR_END 'e'
#define OPT_LENGTH 'l'
#define OPT_OUTPUT 'o'

void PrintUsage() {
    wprintf(L"Usage: memdump [OPTIONS]\n");
    wprintf(L"OPTIONS:\n");
    wprintf(L"  -p PID           : Target process by PID (integer)\n");
    wprintf(L"  -n PROCESS_NAME  : Target process by name (e.g., notepad.exe)\n");
    wprintf(L"  -a START_ADDR    : Memory start address (hex, e.g., 0x1000 or 1000)\n");
    wprintf(L"  -e END_ADDR      : Memory end address (hex, e.g., 0x2000 or 2000)\n");
    wprintf(L"  -l LENGTH        : Memory length in bytes (hex/integer, e.g., 0x1000 or 4096)\n");
    wprintf(L"  -o OUTPUT_FILE   : Output dump file path\n");
    wprintf(L"Examples:\n");
    wprintf(L"  memdump -n notepad.exe -a 0x1000 -l 0x500 -o dump.bin\n");
    wprintf(L"  memdump -n notepad.exe -a 0x1000 -e 0x1500 -o dump.bin\n");
    wprintf(L"  memdump -p 23324 -a 0x1000 -l 0x500 -o dump.bin\n");
}

int wmain(int argc, wchar_t* argv[]) {
    if (argc < 2) {
        PrintUsage();
        return 1;
    }

    DWORD targetPid = 0;
    wchar_t* processName = NULL;
    ULONG_PTR addrStart = 0;
    ULONG_PTR addrEnd = 0;
    SIZE_T length = 0;
    wchar_t* outputFile = NULL;

    int i = 1;
    while (i < argc) {
        if (argv[i][0] == L'-' && argv[i][1] != L'\0' && argv[i][2] == L'\0') {
            switch (towlower(argv[i][1])) {
                case OPT_PID:
                    i++;
                    if (i >= argc) {
                        fwprintf(stderr, L"Error: Option '-%c' requires an argument.\n", OPT_PID);
                        return 1;
                    }
                    targetPid = ParsePid(argv[i]);
                    if (targetPid == 0) {
                        fwprintf(stderr, L"Error: Invalid PID '%ls'.\n", argv[i]);
                        return 1;
                    }
                    break;
                case OPT_NAME:
                    i++;
                    if (i >= argc) {
                        fwprintf(stderr, L"Error: Option '-%c' requires an argument.\n", OPT_NAME);
                        return 1;
                    }
                    processName = argv[i];
                    break;
                case OPT_ADDR_START:
                    i++;
                    if (i >= argc) {
                        fwprintf(stderr, L"Error: Option '-%c' requires an argument.\n", OPT_ADDR_START);
                        return 1;
                    }
                    addrStart = ParseAddress(argv[i]);
                    if (addrStart == 0) {
                        fwprintf(stderr, L"Error: Invalid start address '%ls'.\n", argv[i]);
                        return 1;
                    }
                    break;
                case OPT_ADDR_END:
                    i++;
                    if (i >= argc) {
                        fwprintf(stderr, L"Error: Option '-%c' requires an argument.\n", OPT_ADDR_END);
                        return 1;
                    }
                    addrEnd = ParseAddress(argv[i]);
                    if (addrEnd == 0) {
                        fwprintf(stderr, L"Error: Invalid end address '%ls'.\n", argv[i]);
                        return 1;
                    }
                    break;
                case OPT_LENGTH:
                    i++;
                    if (i >= argc) {
                        fwprintf(stderr, L"Error: Option '-%c' requires an argument.\n", OPT_LENGTH);
                        return 1;
                    }
                    length = ParseSize(argv[i]);
                    if (length == 0) {
                        fwprintf(stderr, L"Error: Invalid length '%ls'.\n", argv[i]);
                        return 1;
                    }
                    break;
                case OPT_OUTPUT:
                    i++;
                    if (i >= argc) {
                        fwprintf(stderr, L"Error: Option '-%c' requires an argument.\n", OPT_OUTPUT);
                        return 1;
                    }
                    outputFile = argv[i];
                    break;
                default:
                    fwprintf(stderr, L"Error: Unknown option '-%c'.\n", argv[i][1]);
                    PrintUsage();
                    return 1;
            }
        }
        i++;
    }

    if (targetPid == 0 && processName == NULL) {
        fwprintf(stderr, L"Error: Process identifier (PID or name) is required.\n");
        PrintUsage();
        return 1;
    }
    if (processName != NULL && targetPid != 0) {
        fwprintf(stderr, L"Error: Cannot specify both PID and process name.\n");
        PrintUsage();
        return 1;
    }
    if (processName != NULL) {
        DWORD pidResult = GetPidByName(processName);
        if (pidResult == 0) {
            fwprintf(stderr, L"Error: Process '%ls' not found.\n", processName);
            return 1;
        } else if (pidResult == (DWORD) -1) {
            fwprintf(stderr, L"Error: Multiple processes named '%ls' found. Use PID instead.\n", processName);
            return 1;
        }
        targetPid = pidResult;
    }
    if (addrStart == 0) {
        fwprintf(stderr, L"Error: Start address is required.\n");
        PrintUsage();
        return 1;
    }
    if ((addrEnd == 0) == (length == 0)) {
        fwprintf(stderr, L"Error: Exactly one of end address (-%c) or length (-%c) must be specified.\n", OPT_ADDR_END, OPT_LENGTH);
        PrintUsage();
        return 1;
    }
    if (outputFile == NULL) {
        fwprintf(stderr, L"Error: Output file (-%c) is required.\n", OPT_OUTPUT);
        PrintUsage();
        return 1;
    }
    if (addrEnd != 0 && addrEnd <= addrStart) {
        fwprintf(stderr, L"Error: End address must be greater than start address.\n");
        return 1;
    }
    if (length != 0 && addrStart + length <= addrStart) {
        fwprintf(stderr, L"Error: Length causes address overflow.\n");
        return 1;
    }
    if (length == 0) {
        length = (SIZE_T) (addrEnd - addrStart);
    }

    HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, targetPid);
    if (hProcess == NULL) {
        DWORD err = GetLastError();
        fwprintf(stderr, L"Error: Cannot open process PID %d. Error code: %d\n", targetPid, err);
        return 1;
    }

    HANDLE hHeap = GetProcessHeap();
    if (!hHeap) {
        DWORD err = GetLastError();
        fwprintf(stderr, L"Error: Failed to get process heap. Error code: %d\n", err);
        CloseHandle(hProcess);
        return 1;
    }

    LPVOID buffer = HeapAlloc(hHeap, 0, length);
    if (!buffer) {
        DWORD err = GetLastError();
        fwprintf(stderr, L"Error: Failed to allocate memory for dump buffer. Error code: %d\n", err);
        CloseHandle(hProcess);
        return 1;
    }

    SIZE_T bytesRead = 0;
    BOOL success = ReadProcessMemory(hProcess, (LPCVOID) addrStart, buffer, length, &bytesRead);
    if (!success) {
        DWORD err = GetLastError();
        fwprintf(stderr, L"Error: Failed to read memory from process %d. Error code: %d\n", targetPid, err);
        HeapFree(hHeap, 0, buffer);
        CloseHandle(hProcess);
        return 1;
    }

    if (bytesRead != length) {
        fwprintf(stderr, L"Warning: Only read %zu bytes out of %zu requested.\n", bytesRead, length);
        length = bytesRead;
    }

    HANDLE hFile = CreateFileW(outputFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        fwprintf(stderr, L"Error: Cannot open output file '%ls' for writing. Error code: %d\n", outputFile, err);
        HeapFree(hHeap, 0, buffer);
        CloseHandle(hProcess);
        return 1;
    }

    DWORD bytesWritten = 0;
    success = WriteFile(hFile, buffer, (DWORD) length, &bytesWritten, NULL);
    if (!success || bytesWritten != length) {
        DWORD err = GetLastError();
        fwprintf(stderr, L"Error: Failed to write all data to file '%ls'. Wrote %lu out of %zu bytes. Error code: %d\n", outputFile, bytesWritten, length, err);
        CloseHandle(hFile);
        HeapFree(hHeap, 0, buffer);
        CloseHandle(hProcess);
        return 1;
    }

    CloseHandle(hFile);
    HeapFree(hHeap, 0, buffer);
    CloseHandle(hProcess);
    return 0;
}
