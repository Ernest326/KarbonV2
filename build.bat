@echo off
set "target=Karbon"
if /I "%1"=="editor" set "target=karbon_editor"
cmake . -G "MinGW Makefiles" && mingw32-make && .\build\%target%