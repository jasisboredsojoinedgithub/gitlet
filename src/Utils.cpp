#include "Utils.h"

std::string Utils::sha1(const std::vector<char>& vals) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(vals.data()), vals.size(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

std::vector<char> Utils::readContents(const std::string& file) {
    std::ifstream ifs(file, std::ios::binary | std::ios::ate);
    if (!ifs) {
        throw std::invalid_argument("must be a normal file");
    }

    auto end = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    std::vector<char> bytes(end - ifs.tellg());

    if (!ifs.read(bytes.data(), bytes.size())) {
        throw std::invalid_argument("could not read file");
    }

    return bytes;
}

void Utils::writeContents(const std::string& file, const std::vector<char>& bytes) {
    std::ofstream ofs(file, std::ios::binary);
    if (!ofs) {
        throw std::invalid_argument("could not open file for writing");
    }

    if (!ofs.write(bytes.data(), bytes.size())) {
        throw std::invalid_argument("could not write to file");
    }
}

fs::path Utils::join(const std::string& first, const std::vector<std::string>& others) {
    fs::path path = first;
    for (const auto& part : others) {
        path /= part;
    }
    return path;
}


std::string Utils::readStringFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

void Utils::writeStringToFile(const std::string& text, const std::string& filepath, bool overwrite) {
    std::ofstream file(filepath, overwrite ? std::ofstream::out : std::ofstream::app);
    file << text;
}