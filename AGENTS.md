# Project Guide for opencode

## Build

```bash
# Windows (MinGW) — tested Qt 6.11.0 / MinGW 13.10
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

## Conventions

- Language: C++17, Qt6 (Widgets + Network), CMake ≥3.16
- PascalCase classes, snake_case files, Qt5-style connect (modern functor syntax)
- `CMAKE_AUTOMOC` and `CMAKE_AUTORCC` enabled
- Config centralised in `AppConfig.hpp` — update `APP_VERSION` here and in `version.json` for releases
- Language files: `lang/<code>.txt`, UTF-8, `key=value` format, `#` comments
- Icons: PNG in `ico/` via `resources.qrc` (prefixed `:/ico/`); PE icon via `app.rc` → `windres`
- Settings: `application.ini` next to executable, auto-regenerated if missing
- No test framework configured

## Build quirks

- `app.rc` is pre-compiled with `windres` at CMake **configure** time (`CMakeLists.txt:14-22`). If `app_icon.o` is missing, reconfigure with `cmake`.
- `windows/` and `linux/` are gitignored build output directories; `lang/` and `ico/app.ico` are copied to the build dir at configure time.

## Key files

| File | Role |
|---|---|
| `AppConfig.hpp` | Central config: version, URLs, icon info, lang base URL |
| `MainWindow.hpp/.cpp` | Main window: menus, toolbar, dock, closeEvent, settings |
| `LangueManager.hpp/.cpp` | Multi-language: load `.txt`, download from GitHub, system detect |
| `UpdateChecker.hpp/.cpp` | Online version check via HTTP + JSON + QVersionNumber (10s timeout) |
| `ComponentToolbox.hpp/.cpp` | Component toolbox tree widget with hardcoded categories |
| `Project.hpp/.cpp` | JSON project load/save (`version`, `name`, `components[]`) |
| `version.json` | Remote version manifest (hosted on GitHub raw) |
| `guide_implantation_fonctionnalites.md` | Full how-to: 6 Qt features + 20 pitfall sections (emojis, SSL, debug, signals, QSS, deploy) |
| `Qt GUI — bibliothèques et technologies utilisées.md` | All Qt classes & technologies by category |
| `langues.md` | Guide: adding/downloading languages |
| `comment_avoir_une_icone.md` | Multi-platform icon guide |
| `verification_mise_a_jour.md` | Update check guide |

## Notes

- Language auto-detect: system locale → `francais`/`anglais`, falls back to `anglais`
- Missing language files auto-downloaded from `LANG_BASE_URL` (GitHub raw)
- `application.ini` auto-recreated if deleted; saves `langue` and `geometry`
- Tested on Windows (MinGW), not yet on Linux
