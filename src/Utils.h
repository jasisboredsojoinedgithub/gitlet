#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

namespace fs = std::filesystem;


class Utils {
public:
    static std::string sha1(const std::vector<char>& vals);

    
    static std::string sha1(const std::string& str);

    static std::string sha1(const fs::path& path);

    static bool restrictedDelete(const std::string& file);

    static std::vector<char> readContents(const std::string& file);

    static void writeContents(const std::string& file, const std::vector<char>& bytes);

    static fs::path join(const std::string& first, const std::vector<std::string>& others);

    static std::string readStringFromFile(const std::string& file);

    static void writeStringToFile(const std::string& file, const std::string& str, bool overwrite);

};

#endif