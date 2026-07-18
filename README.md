# ApplicationVide

Application Qt6 vide — code source unique compilable sous Windows et Linux.

Testé et fonctionnel sous **Windows 10/11** (MinGW).

## Structure

```
ApplicationVide/
├── CMakeLists.txt           # Build système (unique)
├── main.cpp                 # Point d'entrée
├── MainWindow.hpp/.cpp      # Fenêtre principale (menus, toolbar, dock)
├── Project.hpp/.cpp         # Projet JSON (load/save)
├── ComponentToolbox.hpp/.cpp# Boîte à outils de composants
├── resources.qrc            # Ressources (icônes)
├── ico/                     # Icônes PNG 32–256px
├── windows/                 # Build Windows (exécutable + DLLs)
└── linux/                   # Build Linux (sortie)
```

## Prérequis

- **Qt 6** (module Widgets)
- **CMake** ≥ 3.16
- Compilateur C++17

## Compilation

### Windows (MinGW) — testé

```bash
# Depuis la racine du projet :
cmake -S . -B windows -G "MinGW Makefiles" ^
  -DCMAKE_PREFIX_PATH=C:/Qt/6.x.x/mingw_64 ^
  -DCMAKE_CXX_COMPILER=C:/Qt/Tools/mingwXXX_64/bin/g++.exe ^
  -DCMAKE_MAKE_PROGRAM=C:/Qt/Tools/mingwXXX_64/bin/mingw32-make.exe
cmake --build windows
```

### Windows (MSVC)

```bash
cmake -S . -B windows -G "Visual Studio 17 2022" ^
  -DCMAKE_PREFIX_PATH=C:/Qt/6.x.x/msvc2022_64
cmake --build windows --config Release
```

### Linux

```bash
cmake -S . -B linux -DCMAKE_PREFIX_PATH=/opt/Qt/6.x.x/gcc_64
cmake --build linux
```

## Déploiement (Windows)

Après compilation, copier les DLL Qt à côté de l'exécutable :

```bash
windeployqt windows/ApplicationVide.exe
```

## Fonctionnalités

- Menu **Fichier** (📂 Charger / 💾 Sauvegarder projet JSON / ❌ Quitter)
- Barre d'outils (📂 Charger / 💾 Sauvegarder)
- Menu **Outils** → 🧰 Boîte à outils (composants sélectionnables)
- Projet au format JSON (version, name, components[])
