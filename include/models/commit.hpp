#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "../utils/hashUtils.hpp"

class Commit {
public:
    std::string id;
    std::string message;
    std::string author;
    std::string timestamp;
    std::string branch;
    std::vector<std::string> parentIds;
    std::unordered_map<std::string, std::string> fileHashes;  // filepath -> hash

    Commit(const std::string& msg, const std::string& branchName, 
           const std::vector<std::string>& parents = {})
        : id(HashUtils::generateId())
        , message(msg)
        , author("system")
        , timestamp(HashUtils::getCurrentTimestamp())
        , branch(branchName)
        , parentIds(parents) {}

    void addFile(const std::string& path, const std::string& hash) {
        fileHashes[path] = hash;
    }

    bool hasFile(const std::string& path) const {
        return fileHashes.find(path) != fileHashes.end();
    }

    std::string getFileHash(const std::string& path) const {
        auto it = fileHashes.find(path);
        return it != fileHashes.end() ? it->second : "";
    }

    json toJson() const {
        return {
            {"id", id},
            {"message", message},
            {"author", author},
            {"timestamp", timestamp},
            {"branch", branch},
            {"parentIds", parentIds},
            {"fileHashes", fileHashes}
        };
    }

    static Commit fromJson(const json& j) {
        Commit commit(j["message"], j["branch"]);
        commit.id = j["id"];
        commit.author = j["author"];
        commit.timestamp = j["timestamp"];
        commit.parentIds = j["parentIds"].get<std::vector<std::string>>();
        commit.fileHashes = j["fileHashes"].get<std::unordered_map<std::string, std::string>>();
        return commit;
    }
};