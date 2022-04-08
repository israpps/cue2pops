@echo off
cd /d "%~dp0"
for %%a in (%*) do cue2pops.exe %%a
pause