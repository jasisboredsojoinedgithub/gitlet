#include "StagingArea.h"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <sstream>



StagingArea::StagingArea() {}

void StagingArea::add(const std::string& fileName, const std::string& sha1) {
    addedFiles[fileName] = sha1;
}

void StagingArea::addToRemovedFiles(const std::string& fileName) {
    removedFiles.push_back(fileName);
}

void StagingArea::clear() {
    addedFiles.clear();
    removedFiles.clear();
}

std::unordered_map<std::string, std::string> StagingArea::getAddedFiles() const {
    return addedFiles;
}

std::vector<std::string> StagingArea::getRemovedFiles() const {
    return removedFiles;
}

template<class Archive>
void StagingArea::serialize(Archive& archive, const unsigned int version) {
    archive & addedFiles;
    archive & removedFiles;
}

template<class Archive>
void StagingArea::deserialize(Archive& archive, const unsigned int version) {
    archive & addedFiles;
    archive & removedFiles;
}


std::string StagingArea::serializeToString() const {
    std::ostringstream archive_stream;
    boost::archive::text_oarchive archive(archive_stream);
    archive << *this;
    return archive_stream.str();
}

void StagingArea::deserializeFromString(const std::string& str) {
    std::istringstream archive_stream(str);
    boost::archive::text_iarchive archive(archive_stream);
    archive >> *this;
}