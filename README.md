# 2D-engine

2D-engine - this is a basic graphics 2D engine made in C++. (use WASD to move).

## Structure

```text
src/
  main.cpp      - main cycle
  Engine.h      - main types of engine
  Engine.cpp    - window, render, input
  Game.cpp      - game logic
```


##Appearance

![App appearance](https://github.com/poseydonskyHub/2D-engine/blob/main/application_image.png)

## CMake assembly
 
Если установлен Visual Studio Build Tools:

```powershell
cmake -S . -B build
cmake --build build
.\build\Debug\Tiny2D.exe
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
