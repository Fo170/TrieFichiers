# Project Guide for opencode

## Build

```bash
# Linux — system Qt6
cmake -S . -B linux -DCMAKE_PREFIX_PATH=/usr -DCMAKE_BUILD_TYPE=Release
cmake --build linux

# Windows (cross-compilation from Linux)
cmake -S . -B windows \
  -G "Unix Makefiles" \
  -DCMAKE_TOOLCHAIN_FILE=toolchain-mingw64.cmake \
  -DCMAKE_PREFIX_PATH=/home/administrateur/Qt/6.4.2/mingw_64 \
  -DQT_HOST_PATH=/usr \
  -DCMAKE_BUILD_TYPE=Release
cmake --build windows

# Windows native (MinGW)
cmake -S . -B windows -G "MinGW Makefiles" ^
  -DCMAKE_PREFIX_PATH=C:/Qt/6.x.x/mingw_64
cmake --build windows
```

## Deploy (Windows)

```bash
windeployqt windows/TrieFichiers.exe
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
- Emoji: `main.cpp` sets `Noto Color Emoji` as fallback font

## Build quirks

- Cross-compilation: `toolchain-mingw64.cmake` + `QT_HOST_PATH=/usr` for host Qt tools
- `windows/` and `linux/` are gitignored build output directories; `lang/` and `ico/app.ico` are copied to the build dir at configure time

## Key files

| File | Role |
|---|---|
| `AppConfig.hpp` | Central config: version, URLs, icon info, lang base URL |
| `MainWindow.hpp/.cpp` | Main window: menus, toolbar, dock, closeEvent, settings |
| `LangueManager.hpp/.cpp` | Multi-language: load `.txt`, download from GitHub, system detect |
| `UpdateChecker.hpp/.cpp` | Online version check via HTTP + JSON + QVersionNumber (10s timeout) |
| `ComponentToolbox.hpp/.cpp` | Toolbox tree widget (cleanup tools category) |
| `FolderCleaner.hpp/.cpp` | Core logic: count/delete empty files, Thumbs.db, empty dirs |
| `CleanupDialog.hpp/.cpp` | Dialog: analyse + clean in two steps |
| `Project.hpp/.cpp` | JSON project load/save (`version`, `name`, `components[]`) |
| `version.json` | Remote version manifest (hosted on GitHub raw) |
| `toolchain-mingw64.cmake` | CMake toolchain for Windows cross-compilation |
| `guide_implantation_fonctionnalites.md` | Full how-to: 6 Qt features + 20 pitfall sections (emojis, SSL, debug, signals, QSS, deploy) |
| `Qt GUI — bibliothèques et technologies utilisées.md` | All Qt classes & technologies by category |
| `langues.md` | Guide: adding/downloading languages |
| `comment_avoir_une_icone.md` | Multi-platform icon guide |
| `verification_mise_a_jour.md` | Update check guide |
| `application_multiplatforme.md` | Cross-platform build structure & commands |

## Notes

- Language auto-detect: system locale → `francais`/`anglais`, falls back to `anglais`
- Missing language files auto-downloaded from `LANG_BASE_URL` (GitHub raw)
- `application.ini` auto-recreated if deleted; saves `langue` and `geometry`
- Tested on Linux (Ubuntu 24.04, Qt 6.4.2) and cross-compiled for Windows (MinGW)
