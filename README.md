# VCS: Lightweight Version Control System

## Overview
VCS is a simple, cross-platform version control system written in C++. It provides basic version control features similar to Git, but is designed for learning and small projects.

## Features
- Repository initialization (`vcs init`)
- Staging files (`vcs add`)
- Committing changes (`vcs commit`)
- Branch management (`vcs branch`, `vcs checkout`)
- Merging branches (`vcs merge`)
- Reverting commits (`vcs revert`)
- Status and log viewing (`vcs status`, `vcs log`)
- File data is compressed using Huffman coding for storage

## How it Works
- All repository state and data are stored in a `.vcs` directory.
- File data in commits is compressed using Huffman coding.
- Branch and commit metadata are stored as JSON files in `.vcs`.

## Platform Support
- Designed for Linux and Windows (POSIX file operations may require adaptation for Windows).
- Build system: CMake (C++17, OpenSSL, nlohmann_json required)

## Building (Linux/WSL)
```
./build.sh
```
This script will automatically install required dependencies (cmake, g++, openssl, nlohmann-json) if missing.

## Building (Windows)
```
build_win.bat
```
- Requires CMake, Visual Studio Build Tools, and vcpkg (for dependency management).
- The script will check for and install dependencies using vcpkg.

## Testing (Linux/WSL)
```
bash testfolder/test.sh
```

## Testing (Windows)
```
test_win.bat
```

## License
MIT

