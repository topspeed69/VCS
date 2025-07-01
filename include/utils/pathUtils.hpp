#pragma once
#include <string>
#include <vector>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <fstream>

class PathUtils {
public:
    // Create directory with specified path
    static bool createDirectory(const std::string& path) {
        return mkdir(path.c_str(), 0755) == 0;
    }

    // Check if path exists
    static bool exists(const std::string& path) {
        struct stat buffer;
        return (stat(path.c_str(), &buffer) == 0);
    }

    // Check if path is a directory
    static bool isDirectory(const std::string& path) {
        struct stat buffer;
        if (stat(path.c_str(), &buffer) != 0) return false;
        return S_ISDIR(buffer.st_mode);
    }

    // Check if path is a regular file
    static bool isFile(const std::string& path) {
        struct stat buffer;
        if (stat(path.c_str(), &buffer) != 0) return false;
        return S_ISREG(buffer.st_mode);
    }

    // Get current working directory
    static std::string getCurrentPath() {
        char buffer[PATH_MAX];
        if (getcwd(buffer, PATH_MAX) != nullptr) {
            return std::string(buffer);
        }
        throw std::runtime_error("Could not get current working directory");
    }

    // Copy a single file
    static bool copyFile(const std::string& source, const std::string& dest) {
        std::ifstream src(source, std::ios::binary);
        if (!src) return false;

        std::ofstream dst(dest, std::ios::binary);
        if (!dst) return false;

        dst << src.rdbuf();
        return true;
    }

    // Copy directory recursively
    static bool copyDirectory(const std::string& source, const std::string& dest) {
        DIR* dir = opendir(source.c_str());
        if (!dir) return false;

        createDirectory(dest);
        struct dirent* entry;

        while ((entry = readdir(dir)) != nullptr) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
                continue;

            std::string srcPath = source + "/" + entry->d_name;
            std::string dstPath = dest + "/" + entry->d_name;

            if (isDirectory(srcPath)) {
                if (!copyDirectory(srcPath, dstPath)) {
                    closedir(dir);
                    return false;
                }
            } else {
                if (!copyFile(srcPath, dstPath)) {
                    closedir(dir);
                    return false;
                }
            }
        }

        closedir(dir);
        return true;
    }

    // Remove a single file
    static bool removeFile(const std::string& path) {
        return unlink(path.c_str()) == 0;
    }

    // Remove directory recursively
    static bool removeDirectory(const std::string& path) {
        DIR* dir = opendir(path.c_str());
        if (!dir) return false;

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
                continue;

            std::string fullPath = path + "/" + entry->d_name;

            if (isDirectory(fullPath)) {
                removeDirectory(fullPath);
            } else {
                removeFile(fullPath);
            }
        }

        closedir(dir);
        return rmdir(path.c_str()) == 0;
    }

    // List files in directory (non-recursive)
    static std::vector<std::string> listDirectory(const std::string& path) {
        std::vector<std::string> files;
        DIR* dir = opendir(path.c_str());
        if (!dir) return files;

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
                continue;
            files.push_back(entry->d_name);
        }

        closedir(dir);
        return files;
    }

    // List files in directory recursively
    static std::vector<std::string> listRecursiveDirectory(const std::string& path) {
        std::vector<std::string> files;
        DIR* dir = opendir(path.c_str());
        if (!dir) return files;

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
                continue;

            std::string fullPath = path + "/" + entry->d_name;
            if (isDirectory(fullPath)) {
                auto subFiles = listRecursiveDirectory(fullPath);
                for (const auto& file : subFiles) {
                    files.push_back(entry->d_name + std::string("/") + file);
                }
            } else {
                files.push_back(entry->d_name);
            }
        }

        closedir(dir);
        return files;
    }

    // Join path components
    template<typename... Args>
    static std::string joinPath(const std::string& first, Args... args) {
        std::string result = first;
        (joinTwo(result, args), ...);
        return result;
    }

    // Get relative path from base path
    static std::string getRelativePath(const std::string& path, const std::string& basePath) {
        if (path.find(basePath) == 0) {
            std::string relative = path.substr(basePath.length());
            if (!relative.empty() && relative[0] == '/') {
                relative = relative.substr(1);
            }
            return relative;
        }
        return path;
    }

    // Get file size
    static size_t getFileSize(const std::string& path) {
        struct stat buffer;
        if (stat(path.c_str(), &buffer) != 0) return 0;
        return buffer.st_size;
    }

    // Get last modified time
    static time_t getLastModifiedTime(const std::string& path) {
        struct stat buffer;
        if (stat(path.c_str(), &buffer) != 0) return 0;
        return buffer.st_mtime;
    }

    static std::string getDirectory(const std::string& path) {
    size_t pos = path.find_last_of('/');
    return pos == std::string::npos ? "" : path.substr(0, pos);
}

private:
    // Helper for joining two path components
    static void joinTwo(std::string& base, const std::string& part) {
        if (base.empty()) {
            base = part;
            return;
        }
        if (part.empty()) return;
        
        if (base.back() != '/') base += '/';
        if (part.front() == '/') base += part.substr(1);
        else base += part;
    }
};