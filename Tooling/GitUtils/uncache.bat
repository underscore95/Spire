REM This is a batch script which untracks and then retracks files, it is useful because I am having issues where when I go back to an old commit, files that have since
REM git ignored get tracked and then when I go back to main branch they still are tracked

@echo off
cd /d "%~dp0\..\.."

git rm -r --cached .
git add .

pause