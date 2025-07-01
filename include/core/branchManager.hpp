#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "../models/branch.hpp"

class BranchManager {
private:
    std::unordered_map<std::string, std::shared_ptr<Branch>> branches;
    std::string currentBranch;

    void saveBranchState() const;
    void loadBranchState();

public:
    BranchManager();
    ~BranchManager();

    bool createBranch(const std::string& name, const std::string& startCommit = "");
    bool switchBranch(const std::string& name);
    void updateBranchCommit(const std::string& commitId);
    
    std::string getCurrentBranch() const;
    std::string getCurrentCommitId() const;
    std::vector<std::string> getAllBranches() const;
    bool branchExists(const std::string& name) const;
    std::string getBranchCommit(const std::string& branchName) const;
};