# Pentelka

**Author:** Ondřej Hruboš

Pentelka is a lightweight, cross-platform Qt-based drawing and image-editing application designed for quick sketches.
It provides a canvas, multiple drawing tools (brush, spray, fill, text) and image loading/saving.

# Features

* Painting tools: brush, spray, eraser, fill, patterned paint and fill
* Simple text tool
* Load/save images
* Canvas resizing

## Building & Running

Pentelka includes a Makefile that works on **Windows** and **Linux** and automatically uses **CMake + Ninja**.

#### Build

Run `make` in the project root to configure and build the application.

#### Run

Run `make run` to build and launch the application.

On Windows, this also runs `windeployqt6 --qmldir src/qml build/Pentelka.exe`, which copies the required Qt DLLs.

#### Clean

Run `make clean` to remove the build artifacts.

## Building Without the Makefile (optional)

You can also use plain CMake:

1. Configure: `cmake -S src -B build -G Ninja -DCMAKE_BUILD_TYPE=Release`
2. Build: `cmake --build build`

Alternatively, open the project in **Qt Creator** by selecting `CMakeLists.txt` inside the `src/` directory.

## Deployment (Windows)

To create a portable folder, run:

`windeployqt6 --qmldir src/qml build/Pentelka.exe`

This copies all required Qt DLLs and QML modules so the application can be run on other Windows systems.
