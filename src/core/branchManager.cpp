#include "branchManager.hpp"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

BranchManager::BranchManager() {
    loadBranchState();
}

BranchManager::~BranchManager() {
    saveBranchState();
}

void BranchManager::loadBranchState() {
    try {
        std::ifstream file(".vcs/branches.json");
        if (file.is_open()) {
            json j = json::parse(file);
            currentBranch = j["currentBranch"];
            
            for (const auto& [name, branchData] : j["branches"].items()) {
                auto branch = std::make_shared<Branch>(name, branchData["currentCommitId"]);
                branch->commitHistory = branchData["commitHistory"].get<std::vector<std::string>>();
                branches[name] = branch;
            }
        }
    } catch (...) {
        // Initialize with default state if file doesn't exist
        createBranch("main");
    }
}

void BranchManager::saveBranchState() const {
    json j;
    j["currentBranch"] = currentBranch;
    
    json branchesJson;
    for (const auto& [name, branch] : branches) {
        branchesJson[name] = {
            {"name", branch->name},
            {"currentCommitId", branch->currentCommitId},
            {"commitHistory", branch->commitHistory}
        };
    }
    j["branches"] = branchesJson;

    std::ofstream file(".vcs/branches.json");
    file << j.dump(4);
}

bool BranchManager::createBranch(const std::string& name, const std::string& startCommit) {
    if (branches.find(name) != branches.end()) {
        return false;
    }

    branches[name] = std::make_shared<Branch>(name, startCommit);
    if (branches.size() == 1) {
        currentBranch = name;
    }
    saveBranchState();
    return true;
}

bool BranchManager::switchBranch(const std::string& name) {
    if (branches.find(name) == branches.end()) {
        return false;
    }
    
    currentBranch = name;
    saveBranchState();
    return true;
}

void BranchManager::updateBranchCommit(const std::string& commitId) {
    if (auto branch = branches[currentBranch]) {
        branch->currentCommitId = commitId;
        branch->commitHistory.push_back(commitId);
        saveBranchState();
    }
}

std::string BranchManager::getCurrentBranch() const {
    return currentBranch;
}

std::string BranchManager::getCurrentCommitId() const {
    auto it = branches.find(currentBranch);
    return it != branches.end() ? it->second->currentCommitId : "";
}

std::vector<std::string> BranchManager::getAllBranches() const {
    std::vector<std::string> names;
    for (const auto& [name, _] : branches) {
        names.push_back(name);
    }
    return names;
}

bool BranchManager::branchExists(const std::string& name) const {
    return branches.find(name) != branches.end();
}

std::string BranchManager::getBranchCommit(const std::string& branchName) const {
    auto it = branches.find(branchName);
    return it != branches.end() ? it->second->currentCommitId : "";
}