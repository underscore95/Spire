# Project Structure

Spire consists of 3 projects:
- <b>Spire</b> This project contains game engine utilities such as opening window, handling input. It is also a low level abstraction over Vulkan. Most of Spire was developed before my honours project began.
- <b>SpireVoxel</b> This is a voxel renderer library that sits on top of Spire and provides functionality to mesh, render and serialise voxels.
- <b>Game</b> This is an executable that depends on both Spire and SpireVoxel which will contain any game logic. Currently it only contains some test and profiling logic.

Spire and SpireVoxel both have their own unit tests using GTest which are executed on build.

Main CMake file includes debug symbols in ALL builds, make sure to disable this if you don't want this.