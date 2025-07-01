#pragma once
#include "../common.hpp"
#include "../utils/pathUtils.hpp"
#include "../utils/hashUtils.hpp"
#include "commitManager.hpp"
#include "branchManager.hpp"

class VCS {
private:
    CommitManager commitManager;
    BranchManager branchManager;

    struct FileStatus {
        bool isStaged;
        bool isModified;
        bool isUntracked;
        std::string hash;
    };
    std::unordered_map<std::string, FileStatus> fileStatuses;

    bool isInitialized() const {
        return PathUtils::exists(".vcs");
    }

    void checkInitialized() const {
        if (!isInitialized()) {
            throw std::runtime_error("Not a VCS repository");
        }
    }

    void calculateFileStatus(const std::string& filePath, FileStatus& status) {
        std::string relativePath = PathUtils::getRelativePath(filePath, PathUtils::getCurrentPath());
        if (relativePath.find(".vcs") == 0) return;
        bool existsInWorkDir = PathUtils::exists(filePath);
        bool existsInStaging = PathUtils::exists(PathUtils::joinPath(".vcs", "staging_area", relativePath));
        std::string currentCommitId = branchManager.getCurrentCommitId();
        bool existsInCommit = false;
        if (!currentCommitId.empty()) {
            existsInCommit = PathUtils::exists(PathUtils::joinPath(".vcs", "commits", currentCommitId, "data", relativePath));
        }
        status.isUntracked = existsInWorkDir && !existsInStaging && !existsInCommit;
        status.isStaged = existsInStaging;
        if (existsInWorkDir) {
            std::string currentHash = HashUtils::computeSHA256(filePath);
            std::string previousHash;
            if (existsInStaging) {
                previousHash = HashUtils::computeSHA256(PathUtils::joinPath(".vcs", "staging_area", relativePath));
                status.isModified = currentHash != previousHash;
            } else if (existsInCommit) {
                previousHash = HashUtils::computeSHA256(PathUtils::joinPath(".vcs", "commits", currentCommitId, "data", relativePath));
                status.isModified = currentHash != previousHash;
            }
            status.hash = currentHash;
        }
    }

public:
    void init() {
        if (isInitialized()) {
            throw std::runtime_error("Repository already initialized");
        }

        PathUtils::createDirectory(".vcs");
        PathUtils::createDirectory(".vcs/staging_area");
        PathUtils::createDirectory(".vcs/commits");
        
        branchManager.createBranch("main");
        std::cout << GRN "Initialized empty VCS repository" END << std::endl;
    }

    void add(const std::string& path = ".") {
        checkInitialized();

        if (path == ".") {
            for (const auto& entry : PathUtils::listDirectory(PathUtils::getCurrentPath())) {
                if (entry != ".vcs") {
                    std::string fullPath = PathUtils::joinPath(PathUtils::getCurrentPath(), entry);
                    if (PathUtils::isDirectory(fullPath)) {
                        PathUtils::copyDirectory(fullPath, PathUtils::joinPath(".vcs", "staging_area", entry));
                    } else {
                        PathUtils::copyFile(fullPath, PathUtils::joinPath(".vcs", "staging_area", entry));
                    }
                }
            }
            std::cout << GRN "Added all files to staging area" END << std::endl;
        } else {
            if (!PathUtils::exists(path)) {
                throw std::runtime_error("Path does not exist: " + path);
            }

            std::string targetPath = PathUtils::joinPath(".vcs", "staging_area", path);
            if (PathUtils::isDirectory(path)) {
                PathUtils::copyDirectory(path, targetPath);
            } else {
                PathUtils::copyFile(path, targetPath);
            }
            std::cout << GRN "Added '" << path << "' to staging area" END << std::endl;
        }
    }

    void commit(const std::string& message) {
        checkInitialized();
        
        if (message.empty()) {
            throw std::runtime_error("Commit message cannot be empty");
        }

        std::string commitId = commitManager.createCommit(
            message, 
            branchManager.getCurrentBranch()
        );
        
        branchManager.updateBranchCommit(commitId);
        
        // Clear staging area
        PathUtils::removeDirectory(PathUtils::joinPath(".vcs", "staging_area"));
        PathUtils::createDirectory(PathUtils::joinPath(".vcs", "staging_area"));
        
        std::cout << GRN "Created commit " << commitId << END << std::endl;
    }

    void status() {
        checkInitialized();
        fileStatuses.clear();
        std::vector<std::string> allFiles = PathUtils::listRecursiveDirectory(PathUtils::getCurrentPath());
        for (const auto& filePath : allFiles) {
            std::string relativePath = PathUtils::getRelativePath(filePath, PathUtils::getCurrentPath());
            if (relativePath.find(".vcs") == 0) continue;
            FileStatus status = {false, false, true, ""};
            calculateFileStatus(filePath, status);
            fileStatuses[relativePath] = status;
        }
        std::cout << "On branch " << branchManager.getCurrentBranch() << "\n\n";
        bool hasStaged = false;
        std::cout << GRN "Changes to be committed:" END << std::endl;
        for (const auto& [path, status] : fileStatuses) {
            if (status.isStaged) {
                std::cout << GRN "\tmodified: " << path << END << std::endl;
                hasStaged = true;
            }
        }
        if (!hasStaged) std::cout << "\t(no changes staged for commit)\n";
        bool hasModified = false;
        std::cout << "\n" RED "Changes not staged for commit:" END << std::endl;
        for (const auto& [path, status] : fileStatuses) {
            if (status.isModified && !status.isUntracked) {
                std::cout << RED "\tmodified: " << path << END << std::endl;
                hasModified = true;
            }
        }
        if (!hasModified) std::cout << "\t(no modified files)\n";
        bool hasUntracked = false;
        std::cout << "\n" YEL "Untracked files:" END << std::endl;
        for (const auto& [path, status] : fileStatuses) {
            if (status.isUntracked) {
                std::cout << YEL "\t" << path << END << std::endl;
                hasUntracked = true;
            }
        }
        if (!hasUntracked) std::cout << "\t(no untracked files)\n";
    }

    void log() {
        checkInitialized();
        
        auto history = commitManager.getCommitHistory();
        if (history.empty()) {
            std::cout << "No commits yet" << std::endl;
            return;
        }

        for (const auto& commitId : history) {
            auto commit = commitManager.getCommit(commitId);
            if (!commit) continue;

            std::cout << YEL "commit " << commit->id << END << std::endl;
            std::cout << "Author: " << commit->author << std::endl;
            std::cout << "Date:   " << commit->timestamp << std::endl;
            std::cout << "\n    " << commit->message << "\n" << std::endl;
        }
    }

    void branch(const std::string& name = "") {
        checkInitialized();
        
        if (name.empty()) {
            auto branches = branchManager.getAllBranches();
            std::string current = branchManager.getCurrentBranch();
            
            for (const auto& branch : branches) {
                if (branch == current) {
                    std::cout << GRN "* " << branch << END << std::endl;
                } else {
                    std::cout << "  " << branch << std::endl;
                }
            }
        } else {
            if (branchManager.createBranch(name, branchManager.getCurrentCommitId())) {
                std::cout << GRN "Created branch '" << name << "'" END << std::endl;
            } else {
                throw std::runtime_error("Could not create branch");
            }
        }
    }

    void checkout(const std::string& branchName) {
        checkInitialized();
        
        if (!branchManager.branchExists(branchName)) {
            throw std::runtime_error("Branch does not exist");
        }

        if (branchManager.switchBranch(branchName)) {
            std::string commitId = branchManager.getCurrentCommitId();
            if (!commitId.empty()) {
                // Clear working directory (except .git)
                for (const auto& entry : PathUtils::listDirectory(PathUtils::getCurrentPath())) {
                    if (entry != ".vcs") {
                        std::string fullPath = PathUtils::joinPath(PathUtils::getCurrentPath(), entry);
                        if (PathUtils::isDirectory(fullPath)) {
                            PathUtils::removeDirectory(fullPath);
                        } else {
                            PathUtils::removeFile(fullPath);
                        }
                    }
                }

                // Restore files from commit
                commitManager.restoreCommit(commitId, PathUtils::getCurrentPath());
            }
            
            std::cout << GRN "Switched to branch '" << branchName << "'" END << std::endl;
        } else {
            throw std::runtime_error("Could not switch branch");
        }
    }

    void merge(const std::string& sourceBranch) {
        checkInitialized();
        
        if (sourceBranch == branchManager.getCurrentBranch()) {
            throw std::runtime_error("Cannot merge a branch into itself");
        }

        if (!branchManager.branchExists(sourceBranch)) {
            throw std::runtime_error("Branch does not exist");
        }

        std::string commitId = commitManager.createMergeCommit(
            "Merge branch '" + sourceBranch + "'",
            branchManager.getCurrentBranch(),
            branchManager.getBranchCommit(sourceBranch),
            branchManager.getCurrentCommitId()
        );

        branchManager.updateBranchCommit(commitId);
        
        // Update working directory
        commitManager.restoreCommit(commitId, PathUtils::getCurrentPath());
        
        std::cout << GRN "Merged branch '" << sourceBranch << "' into '" 
                  << branchManager.getCurrentBranch() << "'" END << std::endl;
    }

    void revert(const std::string& commitId) {
        checkInitialized();
        
        std::string targetCommitId = (commitId == "HEAD") ? 
            branchManager.getCurrentCommitId() : commitId;

        if (!commitManager.commitExists(targetCommitId)) {
            throw std::runtime_error("Commit does not exist");
        }

        std::string newCommitId = commitManager.createCommit(
            "Revert to " + targetCommitId,
            branchManager.getCurrentBranch(),
            {branchManager.getCurrentCommitId()}
        );

        // Restore the files from target commit
        commitManager.restoreCommit(targetCommitId, PathUtils::getCurrentPath());
        
        // Update branch
        branchManager.updateBranchCommit(newCommitId);
        
        std::cout << GRN <<"Reverted to commit " << targetCommitId << END << std::endl;
    }
};