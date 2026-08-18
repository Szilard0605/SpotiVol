@echo off
python3 dl_premake5.py
pushd %~dp0\..\
call premake5\premake5.exe vs2022
popd
PAUSE