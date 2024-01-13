#include "Repo.h"
#include <boost/serialization/unordered_map.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/string.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "StagingArea.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <algorithm>

namespace fs = std::filesystem;

#include "Utils.h" 

Repo::Repo() {
    workingDir = fs::current_path();
    deserializeStage();
    HEAD = Utils::readStringFromFile(workingDir / ".gitlet/branches/HEAD.txt");
}

void Repo::init() {
    fs::path repoPath = fs::current_path() / ".gitlet";
    fs::path blobsPath = repoPath / "blobs";
    fs::path commitsPath = repoPath / "commits";
    fs::path branchesPath = repoPath / "branches";
    fs::path stagingPath = repoPath / "staging";
    fs::path globalLogPath = repoPath / "global-log";

    // Check if a repository already exists in the current directory
    if (fs::exists(repoPath)) {
        std::cerr << "A gitlet version-control system already exists in the current directory." << std::endl;
        return;
    }

    // Create the directory structure
    fs::create_directories(blobsPath);
    fs::create_directories(commitsPath);
    fs::create_directories(branchesPath);
    fs::create_directories(stagingPath);
    fs::create_directories(globalLogPath);

    // Create the initial commit
    Commit initialCommit("initial commit", {}, ""); // Assuming constructor parameters match this signature
    std::string commitHash = initialCommit.getOwnHash(); // Assuming Commit objects can compute their own hash
    serializeCommit(initialCommit, commitsPath / (commitHash + ".txt"));

    // Set the master branch to point to the initial commit
    fs::path masterBranchPath = branchesPath / "master.txt";
    std::ofstream masterFile(masterBranchPath.string());
    if (!masterFile) {
        std::cerr << "Failed to create the master branch file." << std::endl;
        return;
    }
    masterFile << commitHash;
    masterFile.close();

    // Set the HEAD to point to the master branch
    fs::path headPath = branchesPath / "HEAD.txt";
    std::ofstream headFile(headPath.string());
    if (!headFile) {
        std::cerr << "Failed to set the HEAD." << std::endl;
        return;
    }
    headFile << "master";
    headFile.close();

    // Initialize an empty staging area
    stage = StagingArea(); // Assuming 'stage' is a member of Repo and StagingArea has a default constructor
    serializeStage(); // Assuming serializeStage writes the staging area to a file

    std::cout << "Initialized an empty gitlet repository in " << fs::absolute(repoPath) << std::endl;

    // Create the global log file
    std::ofstream globalLogFile(globalLogPath / "gl.txt");
    globalLogFile.close();

}

std::string Repo::getHEAD() const {
    return HEAD;
}

StagingArea Repo::getStage() const {
    return stage;
}
//if added file is . add all regular file in the flat non-recursive dir
//except Gitlet itself and .gitlet directory    
void Repo::add(const std::string& fileName) {
    if (fileName == ".") {
        for (const auto& file : fs::directory_iterator(workingDir)) {
            if (fs::is_regular_file(file) && file.path().extension() != ".gitlet" && file.path().filename() != "Gitlet") {
                //slicing only the filename from the path
                add(file.path().filename().string());
            }
        }
    } else {
        if (!fs::exists(workingDir / fileName)) {
            std::cout << "File does not exist." << std::endl;
            return;
        }

        std::vector<char> fileContents = Utils::readContents(workingDir / fileName);
        std::string sha1 = Utils::sha1(fileContents);
        std::string blobPath = (workingDir / ".gitlet/blobs" / (sha1 + ".txt")).string();
        if (!fs::exists(blobPath)) {
            Utils::writeContents(blobPath, fileContents);
        }

        stage.add(fileName, sha1);
        serializeStage();
    }
}


void Repo::commitment(const std::string& msg) {
    if (stage.getAddedFiles().empty() && stage.getRemovedFiles().empty()) {
        std::cout << "No changes added to the commit." << std::endl;
        return;
    } else if (msg.empty()) {
        std::cout << "Please enter a commit message." << std::endl;
        return;
    }

    Commit curr = getCurrentCommit();
    std::unordered_map<std::string, std::string> copiedBlobs = curr.getBlobs();
    for (const auto& file : stage.getAddedFiles()) {
        copiedBlobs[file.first] = file.second;
    }
    for (const auto& fileToRemove : stage.getRemovedFiles()) {
        copiedBlobs.erase(fileToRemove);
    }

    Commit newCommit(msg, copiedBlobs, curr.getOwnHash());
    std::string commitPathString = (workingDir / ".gitlet/commits" / (newCommit.getOwnHash() + ".txt")).string();
    std::ofstream ofs(commitPathString);
    boost::archive::text_oarchive oa(ofs);
    oa << newCommit;
    ofs.close();

    std::string branchPathString = (workingDir / ".gitlet/branches" / (HEAD + ".txt")).string();
    std::ofstream branchFile(branchPathString);
    branchFile << newCommit.getOwnHash();
    branchFile.close();

    stage.clear();
    serializeStage();
}

void Repo::rm(const std::string& fileName) {
    bool isStaged = (stage.getAddedFiles().find(fileName) != stage.getAddedFiles().end());
    Commit curr = getCurrentCommit();
    bool isTracked = (curr.getBlobs().find(fileName) != curr.getBlobs().end());

    if (isTracked) {
        fs::remove(workingDir / fileName);
        stage.addToRemovedFiles(fileName);
        if (isStaged) {
            stage.getAddedFiles().erase(fileName);
        }
        serializeStage();
    } else if (isStaged) {
        stage.getAddedFiles().erase(fileName);
        serializeStage();
    } else {
        std::cout << "No reason to remove the file." << std::endl;
    }
}

void Repo::log() const {
    //always show all commits
    Commit curr = getCurrentCommit();
    while (!curr.getOwnHash().empty()) {
        std::cout << curr.globalLog();
        if (curr.getParentHash().empty()) {
            break;
        }
        curr = deserializeCommit(workingDir / ".gitlet/commits" / (curr.getParentHash() + ".txt"));
    }
}

void Repo::global() const {
    std::ifstream inFile(workingDir / ".gitlet/global-log/gl.txt");
    std::string line;
    while (std::getline(inFile, line)) {
        std::cout << line << std::endl;
    }
}

void Repo::find(const std::string& msg) const {
    bool found = false;
    for (const auto& entry : fs::directory_iterator(workingDir / ".gitlet/commits")) {
        Commit currCommit = deserializeCommit(entry.path());
        if (currCommit.getMessage() == msg) {
            std::cout << currCommit.getOwnHash() << std::endl;
            found = true;
        }
    }
    if (!found) {
        std::cout << "Found no commit with that message." << std::endl;
    }
}

void Repo::status() const {
    // List branches
    std::vector<std::string> branches;
    for (const auto& entry : fs::directory_iterator(workingDir / ".gitlet/branches")) {
        std::string branchName = entry.path().filename().string();
        if (branchName != "HEAD.txt") {
            branches.push_back(branchName == HEAD ? "*" + branchName : branchName);
        }
    }
    std::sort(branches.begin(), branches.end());

    // List staged files
    std::vector<std::string> stagedFiles;
    for (const auto& [fileName, _] : stage.getAddedFiles()) {
        stagedFiles.push_back(fileName);
    }
    std::sort(stagedFiles.begin(), stagedFiles.end());

    // List removed files
    std::vector<std::string> removedFiles(stage.getRemovedFiles().begin(), stage.getRemovedFiles().end());
    std::sort(removedFiles.begin(), removedFiles.end());

    // Display status
    std::cout << "=== Branches ===\n";
    for (const auto& branchName : branches) {
        //remove txt extension from branch name
        //read current branch name from HEAD.txt
        
        std::string branch = branchName.substr(0, branchName.size() - 4);
        std::string headBranchPath = workingDir / ".gitlet/branches/HEAD.txt";
        std::string currentBranch = Utils::readStringFromFile(headBranchPath);
        
        if (branch == currentBranch) {
            std::cout << "*";
        } 
        
        std::cout << branch << "\n";
    }
    std::cout << "\n=== Staged Files ===\n";
    for (const auto& file : stagedFiles) {
        std::cout << file << "\n";
    }
    std::cout << "\n=== Removed Files ===\n";
    for (const auto& file : removedFiles) {
        std::cout << file << "\n";
    }
    
    std::cout << "\n=== Modifications Not Staged For Commit ===\n\n=== Untracked Files ===\n";
}

Commit Repo::getCurrentCommit() const {
    std::string headBranchPath = workingDir / ".gitlet/branches/HEAD.txt";
    std::string currentBranch = Utils::readStringFromFile(headBranchPath);
    std::string currentCommitHash = Utils::readStringFromFile(workingDir / ".gitlet/branches" / (currentBranch + ".txt"));
    return deserializeCommit(workingDir / ".gitlet/commits" / (currentCommitHash + ".txt"));
}

void Repo::checkout(const std::vector<std::string>& args) {
    if (args.size() == 1) {
        std::string branchName = args[0];
        fs::path branchPath = workingDir / ".gitlet/branches" / (branchName + ".txt");
        if (!fs::exists(branchPath)) {
            std::cout << "File does not exist in the most recent commit, or no such branch exists." << std::endl;
            return;
        }

        std::string commitID = Utils::readStringFromFile(branchPath);
        Commit commit = deserializeCommit(workingDir / ".gitlet/commits" / (commitID + ".txt"));
        for (const auto& [fileName, blobHash] : commit.getBlobs()) {
            checkoutFile(commit, fileName);
        }

        // Remove files that are not in the commit to reset to
        for (const auto& file : fs::directory_iterator(workingDir)) {
            if (fs::is_regular_file(file) && file.path().filename() != ".gitlet" && file.path().filename() != "Gitlet") {
                std::string relativePath = fs::relative(file.path(), workingDir).string();
                if (commit.getBlobs().find(relativePath) == commit.getBlobs().end()) {
                    fs::remove(file);
                }
            }
        }

        //overwrite current branch'name in HEAD.txt
        std::string headBranchPath = (workingDir / ".gitlet/branches/HEAD.txt").string();
        Utils::writeStringToFile(branchName, headBranchPath, true);

    } else if (args.size() == 3 && args[1] == "--") {
        std::string commitID = args[0];
        std::string fileName = args[2];
        Commit commit = deserializeCommit(workingDir / ".gitlet/commits" / (commitID + ".txt"));
        checkoutFile(commit, fileName);
    } else {
        std::cout << "Incorrect Operands" << std::endl;
    }
}


void Repo::branch(const std::string& branchName) {
    fs::path branchPath = workingDir / ".gitlet/branches" / (branchName + ".txt");
    if (fs::exists(branchPath)) {
        std::cout << "A branch with that name already exists." << std::endl;
        return;
    }

    std::string sha1 = Utils::readStringFromFile((workingDir / ".gitlet/branches" / HEAD).string() + ".txt");
    Utils::writeStringToFile(sha1, branchPath.string(), false);
}

void Repo::rmb(const std::string& branchName) {
    if (branchName == Utils::readStringFromFile(workingDir / ".gitlet/branches/HEAD.txt")) {
        std::cout << "Cannot remove the current branch." << std::endl;
        return;
    }

    fs::path branchPath = workingDir / ".gitlet/branches" / (branchName + ".txt");
    if (fs::remove(branchPath)) {
        std::cout << "Branch " << branchName << " removed." << std::endl;
    } else {
        std::cout << "A branch with that name does not exist." << std::endl;
    }
}

void Repo::reset(const std::string& commitID) {
    Commit commitToReset = deserializeCommit(".gitlet/commits/" + commitID + ".txt");
    if (commitToReset.getOwnHash().empty()) {
        std::cout << "No commit with that id exists." << std::endl;
        return;
    }

    // Checkout files from the commit to reset to
    for (const auto& [fileName, blobHash] : commitToReset.getBlobs()) {
        checkoutFile(commitToReset, fileName);
    }

    // Remove files that are not in the commit to reset to
    for (const auto& file : fs::directory_iterator(workingDir)) {
        if (fs::is_regular_file(file) && file.path().filename() != ".gitlet" && file.path().filename() != "Gitlet") {
            std::string relativePath = fs::relative(file.path(), workingDir).string();
            if (commitToReset.getBlobs().find(relativePath) == commitToReset.getBlobs().end()) {
                fs::remove(file);
            }
        }
    }

    // Update the current branch's commit ID
    std::string headBranchPath = (workingDir / ".gitlet/branches/HEAD.txt").string();
    std::string currentBranch = Utils::readStringFromFile(headBranchPath);
    std::string branchPath = (workingDir / ".gitlet/branches" / currentBranch).string() + ".txt";
    Utils::writeStringToFile(commitID, branchPath, true);

    std::cout << "Reset to commit " << commitID << std::endl;
}
void Repo::merge(const std::string& branchName) {
    std::string currentBranch = Utils::readStringFromFile(workingDir / ".gitlet/branches/HEAD.txt");
    if (currentBranch == branchName) {
        std::cout << "Cannot merge a branch with itself." << std::endl;
        return;
    }

    std::string currentCommitHash = Utils::readStringFromFile(workingDir / ".gitlet/branches" / (currentBranch + ".txt"));
    std::string branchCommitHash = Utils::readStringFromFile(workingDir / ".gitlet/branches" / (branchName + ".txt"));
    Commit currentCommit = deserializeCommit(workingDir / ".gitlet/commits" / (currentCommitHash + ".txt"));
    Commit branchCommit = deserializeCommit(workingDir / ".gitlet/commits" / (branchCommitHash + ".txt"));

    Commit splitPoint = findSplitPoint(currentCommit, branchCommit);
    if (splitPoint.getOwnHash().empty()) {
        std::cout << "Already up-to-date." << std::endl;
        return;
    } else if (splitPoint.getOwnHash() == branchCommit.getOwnHash()) {
        Utils::writeStringToFile(branchName, workingDir / ".gitlet/branches/HEAD.txt", true);
        std::cout << "Current branch fast-forwarded." << std::endl;
        return;
    }

    std::unordered_set<std::string> currentAncestors = getAllAncestors(currentCommit);
    std::unordered_set<std::string> branchAncestors = getAllAncestors(branchCommit);
    std::unordered_set<std::string> splitPointAncestors = getAllAncestors(splitPoint);

    // Check for uncommitted changes
    if (!stage.getAddedFiles().empty() || !stage.getRemovedFiles().empty()) {
        std::cout << "You have uncommitted changes." << std::endl;
        return;
    }

    // Check for untracked files
    for (const auto& file : fs::directory_iterator(workingDir)) {
        if (fs::is_regular_file(file) && file.path().filename() != ".gitlet" && file.path().filename() != "Gitlet") {
            std::string relativePath = fs::relative(file.path(), workingDir).string();
            if (currentCommit.getBlobs().find(relativePath) == currentCommit.getBlobs().end() && branchCommit.getBlobs().find(relativePath) == branchCommit.getBlobs().end()) {
                std::cout << "There is an untracked file in the way; delete it, or add and commit it first." << std::endl;
                return;
            }
        }   
    }   

    // Check for conflicts
    for (const auto& [fileName, currentBlobHash] : currentCommit.getBlobs()) {
        std::string branchBlobHash = branchCommit.getBlobs().find(fileName) != branchCommit.getBlobs().end() ? branchCommit.getBlobs().at(fileName) : "";
        std::string splitPointBlobHash = splitPoint.getBlobs().find(fileName) != splitPoint.getBlobs().end() ? splitPoint.getBlobs().at(fileName) : "";
        if (currentBlobHash != branchBlobHash && currentBlobHash != splitPointBlobHash && branchBlobHash != splitPointBlobHash) {
            std::cout << "Encountered a merge conflict." << std::endl;
            handleConflict(fileName, currentBlobHash, branchBlobHash);
            return;
        }
    }

    // Update the current branch's commit ID
    std::string headBranchPath = (workingDir / ".gitlet/branches/HEAD.txt").string();
    Utils::writeStringToFile(currentBranch, headBranchPath, true);
    std::cout << "Merged " << branchName << " into " << currentBranch << "." << std::endl;
}

void Repo::handleConflict(const std::string& fileName, const std::string& currentBlobHash, const std::string& branchBlobHash) {
    std::string conflictMarkerHead = "<<<<<<< HEAD\n";
    std::string conflictMarkerMid = "=======\n";
    std::string conflictMarkerEnd = ">>>>>>>\n";

    std::string currentContents = currentBlobHash.empty() ? std::string() :
                                  std::string(Utils::readContents(workingDir / ".gitlet/blobs" / (currentBlobHash + ".txt")).begin(),
                                              Utils::readContents(workingDir / ".gitlet/blobs" / (currentBlobHash + ".txt")).end());
    std::string branchContents = branchBlobHash.empty() ? std::string() :
                                 std::string(Utils::readContents(workingDir / ".gitlet/blobs" / (branchBlobHash + ".txt")).begin(),
                                             Utils::readContents(workingDir / ".gitlet/blobs" / (branchBlobHash + ".txt")).end());
    std::string conflictData = conflictMarkerHead + currentContents + conflictMarkerMid + branchContents + conflictMarkerEnd;
    std::vector<char> conflictDataVec(conflictData.begin(), conflictData.end());
    Utils::writeContents(workingDir / fileName, conflictDataVec);
    stage.add(fileName, Utils::sha1(conflictDataVec));
}

void Repo::checkoutFile(const Commit& commit, const std::string& fileName) {
    std::string blobHash = commit.getBlobs().at(fileName);
    std::vector<char> blobData = Utils::readContents(workingDir / ".gitlet/blobs" / (blobHash + ".txt"));
    Utils::writeContents(workingDir / fileName, blobData);
}

Commit Repo::findSplitPoint(const Commit& currentCommit, const Commit& branchCommit) {
    std::unordered_set<std::string> currentAncestors = getAllAncestors(currentCommit);
    std::unordered_set<std::string> branchAncestors = getAllAncestors(branchCommit);

    for (const auto& ancestorHash : currentAncestors) {
        if (branchAncestors.find(ancestorHash) != branchAncestors.end()) {
            return deserializeCommit(workingDir / ".gitlet/commits" / (ancestorHash + ".txt"));
        }
    }

    return Commit(); // Return an empty commit if no common ancestor is found
}

std::unordered_set<std::string> Repo::getAllAncestors(const Commit& commit) {
    std::unordered_set<std::string> ancestors;
    std::queue<std::string> toVisit;
    toVisit.push(commit.getOwnHash());

    while (!toVisit.empty()) {
        std::string currentHash = toVisit.front();
        toVisit.pop();
        ancestors.insert(currentHash);

        Commit currentCommit = deserializeCommit(workingDir / ".gitlet/commits" / (currentHash + ".txt"));
        if (!currentCommit.getParentHash().empty()) {
            toVisit.push(currentCommit.getParentHash());
        }
    }

    return ancestors;
}

void Repo::serializeStage() {
    std::ofstream ofs(".gitlet/staging/stage.txt");
    boost::archive::text_oarchive oa(ofs);
    oa << stage; // Assuming 'stage' is an instance of StagingArea
}

void Repo::deserializeStage() {
    std::ifstream ifs(".gitlet/staging/stage.txt");
    if(ifs.good()) { // Check if the file exists and is not empty
        boost::archive::text_iarchive ia(ifs);
        ia >> stage; // Assuming 'stage' is an instance of StagingArea
    }
}


void Repo::serializeCommit(const Commit& commit, const std::string& path) {
    std::ofstream ofs(path);
    boost::archive::text_oarchive oa(ofs);
    oa << commit;
}

Commit Repo::deserializeCommit(const std::string& path) const {
    Commit commit; 
    std::ifstream ifs(path);
    if(ifs.good()) { 
        boost::archive::text_iarchive ia(ifs);
        ia >> commit;
    }
    return commit;
}