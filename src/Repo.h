#ifndef REPO_H
#define REPO_H

#include <string>
#include <unordered_map>
#include <filesystem>
#include "Commit.h"
#include "StagingArea.h"
#include "Utils.h"
#include <unordered_set> 

namespace fs = std::filesystem;

class Repo {
public:
    Repo();

    void init();
    std::string getHEAD() const;
    StagingArea getStage() const;
    void add(const std::string& fileName);
    void commitment(const std::string& msg);
    void rm(const std::string& fileName);
    void log() const;
    void global() const;
    void find(const std::string& msg) const;
    void status() const;
    void checkout(const std::vector<std::string>& args);
    void branch(const std::string& branchName);
    void rmb(const std::string& branchName);
    void reset(const std::string& commitID);
    void merge(const std::string& bName);
    Commit findSplitPoint(const Commit& currentCommit, const Commit& branchCommit);
    void checkoutFile(const Commit& commit, const std::string& fileName);
    std::unordered_set<std::string> getAllAncestors(const Commit& commit);
    void serializeStage();
    Commit deserializeCommit(const std::string& path) const;
    void serializeCommit(const Commit& commit, const std::string& path);
    void deserializeStage();
    void handleConflict(const std::string& fileName, const std::string& currentBlobHash, const std::string& branchBlobHash);
private:
    std::string HEAD;
    StagingArea stage;
    fs::path workingDir;

    Commit getCurrentCommit() const;
    fs::path findFile(const std::string& fileName, const fs::path& dir) const;
    std::vector<char> addStuff(const std::vector<char>& addThisStuff, const std::vector<char>& newStuffs) const;
};

#endif // REPO_H
