#include "core/vcsClass.hpp"
#include <iostream>
#include <string>
#include <stdexcept>

void printUsage() {
    std::cout << YEL "VCS (Version Control System) - A lightweight version control system\n\n"
              << "Usage:\n"
              << "  vcs init                           - Initialize repository\n"
              << "  vcs add <'.'|'file_name'>         - Add files to staging area\n"
              << "  vcs commit -m 'message'           - Commit staged files\n"
              << "  vcs status                        - Show working tree status\n"
              << "  vcs branch [name]                 - List/create branches\n"
              << "  vcs checkout <branch>             - Switch branches\n"
              << "  vcs merge <branch>                - Merge branch into current\n"
              << "  vcs revert <'HEAD'|commit>        - Revert to commit\n"
              << "  vcs log                          - Show commit history\n" END << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 0;
    }

    VCS vcs;
    std::string command = argv[1];

    try {
        if (command == "init") {
            vcs.init();
        }
        else if (command == "add") {
            if (argc == 2) {
                throw std::runtime_error("Missing file argument\nUsage: vcs add <'.'|'file_name'>");
            }
            if (argc == 3 && std::string(argv[2]) == ".") {
                vcs.add();
            } else {
                for (int i = 2; i < argc; ++i) {
                    vcs.add(argv[i]);
                }
            }
        }
        else if (command == "commit") {
            if (argc != 4 || std::string(argv[2]) != "-m") {
                throw std::runtime_error("Invalid commit format\nUsage: vcs commit -m 'message'");
            }
            vcs.commit(argv[3]);
        }
        else if (command == "status") {
            vcs.status();
        }
        else if (command == "branch") {
            if (argc == 2) {
                vcs.branch();
            } else {
                vcs.branch(argv[2]);
            }
        }
        else if (command == "checkout") {
            if (argc < 3) {
                throw std::runtime_error("Branch name required\nUsage: vcs checkout <branch>");
            }
            vcs.checkout(argv[2]);
        }
        else if (command == "merge") {
            if (argc < 3) {
                throw std::runtime_error("Branch name required\nUsage: vcs merge <branch>");
            }
            vcs.merge(argv[2]);
        }
        else if (command == "revert") {
            if (argc < 3) {
                throw std::runtime_error("Commit ID required\nUsage: vcs revert <'HEAD'|commit>");
            }
            vcs.revert(argv[2]);
        }
        else if (command == "log") {
            vcs.log();
        }
        else {
            std::cout << RED "Unknown command: " << command << END << std::endl;
            printUsage();
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cout << RED "Error: " << e.what() << END << std::endl;
        return 1;
    }

    return 0;
}