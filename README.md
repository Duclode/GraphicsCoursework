# My Themepark Project

Theme Ride Graphics Coursework Project

This project demonstrates real-time rendering using modern OpenGL, including:
- 3D model loading
- Texture mapping
- Lighting systems
- Camera movement
- Shader-based rendering
- Model importing

---

# Features

- OpenGL 3.3 Core Profile rendering
- First-person camera controls
- Phong lighting model
- Directional, point, and spotlight lighting
- Diffuse and specular texture mapping
- 3D model loading using Assimp
- GUI support using Dear ImGui
- Cross-platform CMake build system

---

# How to Build and run:

*** WARNING It doesn't build correctly on windows and only opens a window with the imgui's
I'm not very good at CMake nor do I know know much about programming on Windows/MSVC so
if you want fix it yourself or build it on Linux SORRY :/ ***

Requirements
------------

Build:
- CMake 3.22 or newer
- A C++17 compatible compiler
  - Visual Studio 2022 (Windows)
  - GCC 8+ (Linux)
  - Clang 7+ (Linux/macOS)
- OpenGL 3.3 Core Profile compatible graphics drivers

Third-party libraries:
All third-party dependencies are included in the repository and built automatically with CMake:

| Library | Purpose |
|---------|---------|
| GLAD | OpenGL function loader |
| GLFW | Window creation and input handling |
| GLM | Mathematics library |
| stb_image | Image loading |
| Assimp | 3D model importing |
| Dear ImGui | Immediate mode GUI |

No additional third-party libraries need to be installed before building.

Build
-----

```bash
cmake -B build
cmake --build build
```

Run
---

Linux:
./build/Themepark

Windows:
build\Debug\Themepark.exe
or
build\Release\Themepark.exe

or open the generated Visual Studio solution and build using Visual Studio:
build/Themepark.sln

# Controls

| Key          | Action             |
| ------------ | ------------------ |
| W            | Move forward       |
| A            | Move left          |
| S            | Move backward      |
| D            | Move right         |
| Mouse        | Look around        |
| Scroll wheel | Change camera zoom |
| F            | Toggle flashlight  |
| ESC          | Exit               |
| LEFT ARROW   | rotate ride left   |
| RIGHT ARROW  | rotate ride right  |
