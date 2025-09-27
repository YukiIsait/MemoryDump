#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>

DWORD ParsePid(const wchar_t* str);
ULONG_PTR ParseAddress(const wchar_t* str);
SIZE_T ParseSize(const wchar_t* str);
DWORD GetPidByName(const wchar_t* name);
