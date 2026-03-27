@echo off
cd /d %~dp0
git rm -r --cached .
git add .
pause