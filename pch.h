#pragma once
#include <Windows.h>
#define _WIN32_DCOM
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>
#include <string>
#include <codecvt>
#include <openssl/sha.h>
#include <sstream>
#include <vector>
#include <iomanip> // for std::hex and std::setw
#include <cstddef> // for std::byte
#include "wmi.h"