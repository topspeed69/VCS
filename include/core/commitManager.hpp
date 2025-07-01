#pragma once
#include "../models/commit.hpp"
#include "../utils/pathUtils.hpp"
#include "../utils/hashUtils.hpp"
#include "../utils/huffmanCoder.hpp"
#include "../common.hpp"

class CommitManager {
private:
    std::unordered_map<std::string, std::shared_ptr<Commit>> commits;
    std::string head;
    void saveCommitState() const {
        json j;
        j["head"] = head;
        json commitsJson;
        for (const auto& [id, commit] : commits) {
            commitsJson[id] = commit->toJson();
        }
        j["commits"] = commitsJson;
        std::ofstream file(PathUtils::joinPath(".vcs", "commits.json"));
        file << j.dump(4);
    }
    void loadCommitState() {
        try {
            std::ifstream file(PathUtils::joinPath(".vcs", "commits.json"));
            if (file.is_open()) {
                json j = json::parse(file);
                head = j["head"];
                for (const auto& [id, commitData] : j["commits"].items()) {
                    commits[id] = std::make_shared<Commit>(Commit::fromJson(commitData));
                }
            }
        } catch (...) {
            head = "";
        }
    }
    std::string computeFileHash(const std::string& filePath) const {
        std::ifstream file(filePath, std::ios::binary);
        if (!file) return "";
        std::stringstream buffer;
        buffer << file.rdbuf();
        return HashUtils::computeSHA256(buffer.str());
    }
    static std::string getDirectory(const std::string& path) {
        size_t pos = path.find_last_of('/');
        return pos == std::string::npos ? "" : path.substr(0, pos);
    }
    std::string findMergeBase(const std::string& commit1, const std::string& commit2) {
        if (commit1.empty() || commit2.empty()) return "";
        std::unordered_set<std::string> ancestors1;
        std::string current = commit1;
        while (!current.empty()) {
            ancestors1.insert(current);
            auto commit = getCommit(current);
            if (!commit || commit->parentIds.empty()) break;
            current = commit->parentIds[0];
        }
        current = commit2;
        while (!current.empty()) {
            if (ancestors1.count(current) > 0) return current;
            auto commit = getCommit(current);
            if (!commit || commit->parentIds.empty()) break;
            current = commit->parentIds[0];
        }
        return "";
    }
    void mergeFiles(const std::string& baseCommit, const std::string& sourceCommit, 
                const std::string& targetCommit, const std::string& outputPath,
                const std::string& sourceBranch) {
        std::string basePath = baseCommit.empty() ? "" : PathUtils::joinPath(".vcs", "commits", baseCommit, "data");
        std::string sourcePath = PathUtils::joinPath(".vcs", "commits", sourceCommit, "data");
        std::string targetPath = PathUtils::joinPath(".vcs", "commits", targetCommit, "data");
        std::unordered_set<std::string> allFiles;
        if (!baseCommit.empty() && PathUtils::exists(basePath)) {
            for (const auto& file : PathUtils::listRecursiveDirectory(basePath)) {
                allFiles.insert(file);
            }
        }
        if (PathUtils::exists(sourcePath)) {
            for (const auto& file : PathUtils::listRecursiveDirectory(sourcePath)) {
                allFiles.insert(file);
            }
        }
        if (PathUtils::exists(targetPath)) {
            for (const auto& file : PathUtils::listRecursiveDirectory(targetPath)) {
                allFiles.insert(file);
            }
        }
        PathUtils::createDirectory(outputPath);
        for (const std::string& file : allFiles) {
            std::string baseFile = baseCommit.empty() ? "" : PathUtils::joinPath(basePath, file);
            std::string sourceFile = PathUtils::joinPath(sourcePath, file);
            std::string targetFile = PathUtils::joinPath(targetPath, file);
            std::string outputFile = PathUtils::joinPath(outputPath, file);
            bool baseExists = !baseCommit.empty() && PathUtils::exists(baseFile);
            bool sourceExists = PathUtils::exists(sourceFile);
            bool targetExists = PathUtils::exists(targetFile);
            std::string outputDir = getDirectory(outputFile);
            if (!outputDir.empty()) PathUtils::createDirectory(outputDir);
            if (!sourceExists && !targetExists) continue;
            if (sourceExists && !targetExists) { PathUtils::copyFile(sourceFile, outputFile); continue; }
            if (!sourceExists && targetExists) { PathUtils::copyFile(targetFile, outputFile); continue; }
            if (areFilesIdentical(sourceFile, targetFile)) { PathUtils::copyFile(sourceFile, outputFile); continue; }
            if (baseExists) {
                std::string baseContent = readFileContent(baseFile);
                std::string sourceContent = readFileContent(sourceFile);
                std::string targetContent = readFileContent(targetFile);
                if (sourceContent == baseContent) { PathUtils::copyFile(targetFile, outputFile); }
                else if (targetContent == baseContent) { PathUtils::copyFile(sourceFile, outputFile); }
                else { handleConflict(sourceFile, targetFile, outputFile, sourceBranch); }
            } else { handleConflict(sourceFile, targetFile, outputFile, sourceBranch); }
        }
    }
    void handleConflict(const std::string& sourceFile, const std::string& targetFile,
                   const std::string& outputFile, const std::string& sourceBranch) {
        if (!sourceFile.empty() && !targetFile.empty() && areFilesIdentical(sourceFile, targetFile)) {
            PathUtils::copyFile(sourceFile, outputFile); return;
        }
        std::string targetContent = readFileContent(targetFile);
        std::string sourceContent = readFileContent(sourceFile);
        if (sourceContent.empty() && !targetContent.empty()) { PathUtils::copyFile(targetFile, outputFile); return; }
        if (!sourceContent.empty() && targetContent.empty()) { PathUtils::copyFile(sourceFile, outputFile); return; }
        std::ofstream out(outputFile);
        bool hasConflict = !sourceContent.empty() && !targetContent.empty() && sourceContent != targetContent;
        if (hasConflict) {
            out << "<<<<<<< HEAD\n";
            out << targetContent;
            if (!targetContent.empty() && targetContent.back() != '\n') out << '\n';
            out << "=======\n";
            out << sourceContent;
            if (!sourceContent.empty() && sourceContent.back() != '\n') out << '\n';
            out << ">>>>>>> " << sourceBranch << "\n";
        } else if (!sourceContent.empty()) {
            out << sourceContent;
        } else if (!targetContent.empty()) {
            out << targetContent;
        }
    }
    void storeCommitFiles(const std::string& commitId, const std::string& sourcePath) {
        std::string commitPath = PathUtils::joinPath(".vcs", "commits", commitId);
        PathUtils::createDirectory(commitPath);
        std::string dataPath = PathUtils::joinPath(commitPath, "data");
        PathUtils::createDirectory(dataPath);
        auto files = PathUtils::listRecursiveDirectory(sourcePath);
        for (const auto& file : files) {
            std::string srcFile = PathUtils::joinPath(sourcePath, file);
            std::string destFile = PathUtils::joinPath(dataPath, file + ".huff");
            std::string compressed = HuffmanCoder::compress(srcFile);
            std::ofstream out(destFile, std::ios::binary);
            out << compressed;
        }
    }
    void restoreCommitFiles(const std::string& commitId, const std::string& destPath) {
        std::string commitPath = PathUtils::joinPath(".vcs", "commits", commitId, "data");
        auto files = PathUtils::listRecursiveDirectory(commitPath);
        for (const auto& file : files) {
            if (file.size() > 5 && file.substr(file.size() - 5) == ".huff") {
                std::string compressedFile = PathUtils::joinPath(commitPath, file);
                std::ifstream in(compressedFile, std::ios::binary);
                std::string compressed((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
                std::string decompressed = HuffmanCoder::decompress(compressed);
                std::string outFile = PathUtils::joinPath(destPath, file.substr(0, file.size() - 5));
                std::ofstream out(outFile, std::ios::binary);
                out << decompressed;
            }
        }
    }
public:
    CommitManager() { loadCommitState(); }
    ~CommitManager() { saveCommitState(); }
    std::string createCommit(const std::string& message, const std::string& branch,
                            const std::vector<std::string>& parents = {}) {
        auto commit = std::make_shared<Commit>(message, branch, parents);
        std::string stagingPath = PathUtils::joinPath(".vcs", "staging_area");
        auto files = PathUtils::listRecursiveDirectory(stagingPath);
        for (const auto& file : files) {
            std::string fullPath = PathUtils::joinPath(stagingPath, file);
            commit->addFile(file, computeFileHash(fullPath));
        }
        storeCommitFiles(commit->id, stagingPath);
        commits[commit->id] = commit;
        head = commit->id;
        saveCommitState();
        return commit->id;
    }
    bool commitExists(const std::string& commitId) const {
        return commits.find(commitId) != commits.end();
    }
    std::shared_ptr<Commit> getCommit(const std::string& commitId) const {
        auto it = commits.find(commitId);
        return it != commits.end() ? it->second : nullptr;
    }
    std::vector<std::string> getCommitHistory(const std::string& startCommit = "") const {
        std::vector<std::string> history;
        std::string current = startCommit.empty() ? head : startCommit;
        while (!current.empty()) {
            history.push_back(current);
            auto commit = getCommit(current);
            if (!commit || commit->parentIds.empty()) break;
            current = commit->parentIds[0];
        }
        return history;
    }
    bool restoreCommit(const std::string& commitId, const std::string& targetPath) {
        auto commit = getCommit(commitId);
        if (!commit) return false;

        std::string commitPath = PathUtils::joinPath(".vcs", "commits", commitId, "data");
        return PathUtils::copyDirectory(commitPath, targetPath);
    }
    std::string createMergeCommit(const std::string& message, const std::string& branch,
                                 const std::string& sourceBranchCommit, 
                                 const std::string& targetBranchCommit) {
        std::string mergeBase = findMergeBase(sourceBranchCommit, targetBranchCommit);
        std::string tempDir = PathUtils::joinPath(".vcs", "merge_temp");
        if (PathUtils::exists(tempDir)) {
            PathUtils::removeDirectory(tempDir);
        }
        PathUtils::createDirectory(tempDir);
        std::string sourceBranch = message.substr(message.find("'") + 1);
        sourceBranch = sourceBranch.substr(0, sourceBranch.find("'"));
        mergeFiles(mergeBase, sourceBranchCommit, targetBranchCommit, tempDir, sourceBranch);
        auto commit = std::make_shared<Commit>(message, branch, 
            std::vector<std::string>{targetBranchCommit, sourceBranchCommit});
        std::string commitPath = PathUtils::joinPath(".vcs", "commits", commit->id);
        PathUtils::createDirectory(commitPath);
        PathUtils::copyDirectory(tempDir, PathUtils::joinPath(commitPath, "data"));
        PathUtils::removeDirectory(tempDir);
        commits[commit->id] = commit;
        head = commit->id;
        saveCommitState();
        return commit->id;
    }
    bool areFilesIdentical(const std::string& file1, const std::string& file2) {
        return computeFileHash(file1) == computeFileHash(file2);
    }
    std::string readFileContent(const std::string& filePath) {
        if (!PathUtils::exists(filePath)) return "";
        std::ifstream file(filePath);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
};