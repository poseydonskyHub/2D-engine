# 2D-engine

2D-engine - this is a basic graphics 2D engine made in C++. (use WASD to move).
!NOW AVAILABLE ON LINUX AND WINDOWS!


## Structure

```text
src/
  main.cpp      - main cycle
  Engine.h      - main types of engine
  Engine.cpp    - window, render, input
  Game.cpp      - game logic
  PlatformWin32.cpp - Windows window backend
  PlatformLinux.cpp - Linux X11 window backend
```


## Appearance

windows
![App appearance](https://github.com/poseydonskyHub/2D-engine/blob/main/application_image.png)

linux (test on arch)
![App appearance](https://github.com/poseydonskyHub/2D-engine/blob/main/linux_apperance.png)

## CMake assembly
 
if installed Visual Studio Build Tools:

```powershell
cmake -S . -B build
cmake --build build
.\build\Debug\Tiny2D.exe
```


## Linux assembly

```powershell
sudo apt install build-essential cmake libx11-dev
cmake -S . -B build
cmake --build build
./build/Tiny2D
```


## Visual Studio launch

open `*.sln` file with project:

```text
Tiny2D.sln
```

Choose `Debug` and `x64`, press `F5` or green arrow-shaped icon.


## Upcoming innovations

1. `Camera2D` - camera
2. `Texture` - picture launch
3. `SpriteBatch` - sprite rendering
4. `Scene` - all game sprites
5. `Entity` + `Component` - ECS-architecture
6. `TileMap` - map made of tiles
