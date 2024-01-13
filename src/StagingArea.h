#ifndef STAGINGAREA_H
#define STAGINGAREA_H

#include <unordered_map>
#include <vector>
#include <string>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <sstream>


class StagingArea {
public:
    StagingArea();
    void add(const std::string& fileName, const std::string& sha1);
    void addToRemovedFiles(const std::string& fileName);
    void clear();

    std::unordered_map<std::string, std::string> getAddedFiles() const;
    std::vector<std::string> getRemovedFiles() const;

    template<class Archive>
    void serialize(Archive& archive, const unsigned int version);

    template<class Archive>
    void deserialize(Archive& archive, const unsigned int version);


    std::string serializeToString() const;

    void deserializeFromString(const std::string &str);

private:
    std::unordered_map<std::string, std::string> addedFiles;
    std::vector<std::string> removedFiles;



};

#endif // STAGINGAREA_H
