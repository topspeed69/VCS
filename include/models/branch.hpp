#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Branch {
public:
    std::string name;
    std::string currentCommitId;
    std::vector<std::string> commitHistory;

    Branch(const std::string& branchName, const std::string& initialCommit = "")
        : name(branchName)
        , currentCommitId(initialCommit) {
        if (!initialCommit.empty()) {
            commitHistory.push_back(initialCommit);
        }
    }

    void addCommit(const std::string& commitId) {
        currentCommitId = commitId;
        commitHistory.push_back(commitId);
    }

    json toJson() const {
        return {
            {"name", name},
            {"currentCommitId", currentCommitId},
            {"commitHistory", commitHistory}
        };
    }

    static Branch fromJson(const json& j) {
        Branch branch(j["name"], j["currentCommitId"]);
        branch.commitHistory = j["commitHistory"].get<std::vector<std::string>>();
        return branch;
    }
};