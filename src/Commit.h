#ifndef COMMIT_H
#define COMMIT_H

#include <string>
#include <unordered_map>
#include <chrono>
#include <sstream>
#include <iomanip>

#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

class Commit {
public:
    Commit();
    Commit(const std::string& msg, const std::unordered_map<std::string, std::string>& blobMap, const std::string& parent);

    std::string calcHash() const;
    std::string getOwnHash() const;
    std::string getParentHash() const;
    std::string getMessage() const;
    std::string getDatetime() const;
    std::unordered_map<std::string, std::string> getBlobs() const;
    std::string globalLog() const;

private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & ownHash;
        ar & parentHash;
        ar & message;
        ar & datetime;
        ar & blobs;
    }

    std::string ownHash;
    std::string parentHash;
    std::string message;
    std::string datetime;
    std::unordered_map<std::string, std::string> blobs; // <fileName, SHA1>

    std::string currentDateTime() const;
};

#endif // COMMIT_H
