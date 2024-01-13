#include "Commit.h"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <fstream>
#include <sstream>
#include <chrono>

#include "Utils.h"

Commit::Commit() {}

Commit::Commit(const std::string& msg, const std::unordered_map<std::string, std::string>& blobMap, const std::string& parent)
    : message(msg), blobs(blobMap), parentHash(parent) {
    datetime = currentDateTime();
    ownHash = calcHash();
}

std::string Commit::calcHash() const {
    std::ostringstream archive_stream;
    boost::archive::text_oarchive archive(archive_stream);
    archive << *this;
    std::string serializedCommit = archive_stream.str();
    std::vector<char> commitData(serializedCommit.begin(), serializedCommit.end());
    return Utils::sha1(commitData);
}

std::string Commit::getOwnHash() const {
    return ownHash;
}

std::string Commit::getParentHash() const {
    return parentHash;
}

std::string Commit::getMessage() const {
    return message;
}

std::string Commit::getDatetime() const {
    return datetime;
}

std::unordered_map<std::string, std::string> Commit::getBlobs() const {
    return blobs;
}

std::string Commit::globalLog() const {
    std::ostringstream log;
    log << "===\n"
        << "Commit " << ownHash << "\n"
        << datetime << "\n"
        << message << "\n\n";
    return log.str();
}

std::string Commit::currentDateTime() const {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    return ss.str();
}
