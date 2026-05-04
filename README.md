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


## Camera 2D

`Camera2D` converts world coordinates into screen coordinates.

Use camera rendering for objects that live in the game world:

```cpp
renderer.drawSprite(player, camera);
```

Use normal rendering for UI that must stay fixed on the screen:

```cpp
renderer.drawRect(Rect{10.0f, 10.0f, 100.0f, 30.0f}, Color::rgb(255, 255, 255));
```


## Texture

An image is a file on disk, for example `assets/player.bmp`.

A texture is image data already loaded into engine memory. The renderer can draw textures, not files directly.

A `Sprite` can either be a colored rectangle or use a texture.

Create a procedural texture:

```cpp
Texture texture = Texture::createChecker(
    16,
    16,
    4,
    Color::rgb(245, 95, 80),
    Color::rgb(255, 205, 120)
);
```

Attach it to a sprite:

```cpp
sprite.texture = &texture;
```

Render it as usual:

```cpp
renderer.drawSprite(sprite, camera);
```

Load your own PNG file:

```cpp
Texture texture = Texture::loadFromFile("assets/player.png");

if (texture.isValid()) {
    sprite.texture = &texture;
}
```

PNG loading uses `stb_image`. BMP loading is still available through `Texture::loadFromBmp(...)`.

The image resolution and sprite size are different things:

```text
image resolution: how many pixels are stored in the file
sprite size: how large the object is drawn in the game world
```

For example, a `200x200` PNG can be drawn as `48x48`, `200x200`, or `500x500`.
The current player sprite is drawn as:

```cpp
player_.size = Vec2{200.0f, 200.0f};
```


## SpriteBatch

`SpriteBatch` is a queue for sprites that should be drawn together.

Without a batch:

```cpp
renderer.drawSprite(target, camera);
renderer.drawSprite(player, camera);
```

With a batch:

```cpp
spriteBatch.begin(camera);
spriteBatch.draw(target);
spriteBatch.draw(player);
spriteBatch.flush(renderer);
```


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


1. `Scene` - all game sprites
2. `Entity` + `Component` - ECS-architecture
