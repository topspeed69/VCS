#pragma once

// Standard library includes
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <unordered_map>

// External libraries
#include <nlohmann/json.hpp>
#define OPENSSL_SUPPRESS_DEPRECATED
#include <openssl/sha.h>

// Color definitions
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define END   "\x1B[0m"

// Using declarations
using json = nlohmann::json;
using std::string;
using std::vector;
using std::unordered_map;
using std::cout;
using std::cerr;
using std::endl;


// Forward declarations
class CommitNode;
class CommitNodeList;
class BranchManager;