# Project Guide for opencode

## Build commands

```bash
# Windows (MinGW) — testé Qt 6.11.0 / MinGW 13.10
cmake -S . -B windows -G "MinGW Makefiles" ^
  -DCMAKE_PREFIX_PATH=C:/Qt/6.x.x/mingw_64 ^
  -DCMAKE_CXX_COMPILER=C:/Qt/Tools/mingwXXX_64/bin/g++.exe ^
  -DCMAKE_MAKE_PROGRAM=C:/Qt/Tools/mingwXXX_64/bin/mingw32-make.exe
cmake --build windows

# Windows (MSVC)
cmake -S . -B windows -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH=C:/Qt/6.x.x/msvc2022_64
cmake --build windows --config Release

# Linux
cmake -S . -B linux -DCMAKE_PREFIX_PATH=/opt/Qt/6.x.x/gcc_64
cmake --build linux
```

## Deploy (Windows)

```bash
windeployqt windows/ApplicationVide.exe
```

## Project conventions

- Language: C++17
- UI Framework: Qt6 Widgets
- Naming: PascalCase for classes, snake_case for files
- Signals/slots: Qt5 style connect
- Icons: PNG in `ico/`, registered in `resources.qrc`
- Project file format: JSON
- Emojis in UI: 📂 💾 ❌ 🧰

## Current status

- Compile et s'exécute sous Windows (MinGW)
- Pas encore testé sous Linux
