#pragma once
#include <string>
#include <regex>
#include "md5.h"

namespace Utils
{
    // Validate Username: 8 to 20 alphanumeric characters or underscore
    inline bool IsValidUsername(const std::string& username)
    {
        static const std::regex pattern("^[a-zA-Z0-9_]{8,20}$");
        return std::regex_match(username, pattern);
    }

    // Validate Password: 8 to 32 characters, no spaces
    inline bool IsValidPassword(const std::string& password)
    {
        static const std::regex pattern("^[^\\s]{8,32}$");
        return std::regex_match(password, pattern);
    }

    // Generate MD5 hash using stbrumme/hash-library
    inline std::string GetMD5Hash(const std::string& input)
    {
        MD5 md5;
        return md5(input);
    }
}