cmake_minimum_required(VERSION 3.10)
project(jolt)
add_subdirectory(JoltPhysics/Build)
add_library(jolt ALIAS Jolt)