// pch.h: Precompiled header file
#pragma once

#ifndef PCH_H
#define PCH_H

// 1. Core MFC Framework (Includes Windows API headers internally)
#include "framework.h"

// 2. Win32 CNG Security Header (safe to include after framework.h)
#include <bcrypt.h>

// 3. Standard C++ Libraries
#include <fstream>
#include <string>
#include <vector>
#include <regex>

// 4. Third-Party Libraries
#include "json.hpp"
using json = nlohmann::json;

// Automatically link BCrypt library across project
#pragma comment(lib, "bcrypt.lib")

std::string HashPasswordSHA256(const std::string& password);

#endif // PCH_H