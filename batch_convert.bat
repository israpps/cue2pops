@echo off
cd /d "%~dp0"
for %%a in (*.cue) do cue2pops.exe %%a
pause