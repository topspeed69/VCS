#!/bin/bash

# Create a directory to test VCS and change into it
TEST_DIR="vcs_test_repo"
LOG_FILE="vcs_test_log.txt"
VCS_BIN="../build/vcs"

# Clean previous test directory and log file
rm -rf "$TEST_DIR"
rm -f "$LOG_FILE"

# Initialize a new VCS repository
echo "Initializing repository..." | tee -a "$LOG_FILE"
mkdir "$TEST_DIR" && cd "$TEST_DIR"
$VCS_BIN init 2>&1 | tee -a "$LOG_FILE"

# Create test files
echo "Creating test files..." | tee -a "$LOG_FILE"
echo "Hello World" > file1.txt
echo "Another file" > file2.txt

# Add files to staging area
echo "Adding files to staging area..." | tee -a "$LOG_FILE"
$VCS_BIN add file1.txt 2>&1 | tee -a "$LOG_FILE"
$VCS_BIN add file2.txt 2>&1 | tee -a "$LOG_FILE"

# Check status
echo "Checking VCS status..." | tee -a "$LOG_FILE"
$VCS_BIN status 2>&1 | tee -a "$LOG_FILE"

# Commit changes
echo "Committing changes..." | tee -a "$LOG_FILE"
$VCS_BIN commit -m "Test commit" 2>&1 | tee -a "$LOG_FILE"

# Check status
echo "Checking VCS status..." | tee -a "$LOG_FILE"
$VCS_BIN status 2>&1 | tee -a "$LOG_FILE"

# Branch creation and checkout
echo "Testing branch creation and checkout..." | tee -a "$LOG_FILE"
$VCS_BIN branch testbranch 2>&1 | tee -a "$LOG_FILE"
$VCS_BIN checkout testbranch 2>&1 | tee -a "$LOG_FILE"

# Making changes in the test branch
echo "Making changes in test branch..." | tee -a "$LOG_FILE"
echo "Test branch file" > branchfile.txt
$VCS_BIN add branchfile.txt 2>&1 | tee -a "$LOG_FILE"
$VCS_BIN commit -m "Branch commit" 2>&1 | tee -a "$LOG_FILE"

# Check status
echo "Checking VCS status..." | tee -a "$LOG_FILE"
$VCS_BIN status 2>&1 | tee -a "$LOG_FILE"

# Merging test branch into main
echo "Merging test branch into main..." | tee -a "$LOG_FILE"
$VCS_BIN checkout main 2>&1 | tee -a "$LOG_FILE"
$VCS_BIN merge testbranch 2>&1 | tee -a "$LOG_FILE"

# Check status
echo "Checking VCS status..." | tee -a "$LOG_FILE"
$VCS_BIN status 2>&1 | tee -a "$LOG_FILE"

# Testing revert functionality
echo "Testing revert functionality..." | tee -a "$LOG_FILE"
$VCS_BIN revert "test_commit" 2>&1 | tee -a "$LOG_FILE"  # Invalid commit hash

# Log
echo "Logging..." | tee -a "$LOG_FILE"
$VCS_BIN log 2>&1 | tee -a "$LOG_FILE"

# End of test
echo "Testing complete. Logs are stored in $LOG_FILE."
