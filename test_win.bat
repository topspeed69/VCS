@echo off
REM Windows test script for VCS
REM Assumes vcs.exe is built and in the project root
setlocal enabledelayedexpansion
set VCS_BIN=build\vcs.exe
set TEST_DIR=vcs_test_repo
set LOG_FILE=vcs_test_log.txt
if exist %TEST_DIR% rmdir /s /q %TEST_DIR%
if exist %LOG_FILE% del %LOG_FILE%
mkdir %TEST_DIR%
cd %TEST_DIR%
echo Creating test files... >> ..\%LOG_FILE%
echo Hello World > file1.txt
echo Another file > file2.txt
..\%VCS_BIN% init >> ..\%LOG_FILE% 2>&1
..\%VCS_BIN% add file1.txt >> ..\%LOG_FILE% 2>&1
..\%VCS_BIN% add file2.txt >> ..\%LOG_FILE% 2>&1
..\%VCS_BIN% status >> ..\%LOG_FILE% 2>&1
..\%VCS_BIN% commit -m "Test commit" >> ..\%LOG_FILE% 2>&1
..\%VCS_BIN% status >> ..\%LOG_FILE% 2>&1
echo Testing branch creation and checkout... >> ..\%LOG_FILE%
..\%VCS_BIN% branch testbranch >> ..\%LOG_FILE% 2>&1
..\%VCS_BIN% checkout testbranch >> ..\%LOG_FILE% 2>&1
echo Test branch file > branchfile.txt
..\%VCS_BIN% add branchfile.txt >> ..\%LOG_FILE% 2>&1
..\%VCS_BIN% commit -m "Branch commit" >> ..\%LOG_FILE% 2>&1
..\%VCS_BIN% status >> ..\%LOG_FILE% 2>&1
..\%VCS_BIN% checkout main >> ..\%LOG_FILE% 2>&1
..\%VCS_BIN% merge testbranch >> ..\%LOG_FILE% 2>&1
..\%VCS_BIN% status >> ..\%LOG_FILE% 2>&1
echo Testing revert functionality... >> ..\%LOG_FILE%
..\%VCS_BIN% revert "test_commit" >> ..\%LOG_FILE% 2>&1
..\%VCS_BIN% log >> ..\%LOG_FILE% 2>&1
echo Testing complete. Logs are stored in %LOG_FILE%.
cd ..
