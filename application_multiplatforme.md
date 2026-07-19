# Application Qt multi-plateforme (Windows + Linux)

## Principe

Qt est multi-plateforme par nature : un même code C++ se compile sur Windows et Linux sans modification. Les seules différences concernent le déploiement (icône PE, empaquetage) et quelques appels système. Ce projet suit cette approche.

## Structure des répertoires

```
projet/
├── CMakeLists.txt          # Fichier unique, pas de plateforme cible
├── main.cpp                # Point d'entrée commun
├── AppConfig.hpp           # Configuration centralisée
├── MainWindow.hpp/.cpp     # Fenêtre principale
├── Project.hpp/.cpp        # Projet JSON
├── ComponentToolbox.hpp/.cpp
├── UpdateChecker.hpp/.cpp
├── LangueManager.hpp/.cpp
├── resources.qrc           # Ressources Qt communes
├── app.rc                  # Ressource Windows PE (ignorée sur Linux)
├── ico/
│   ├── app-32.png          # Icônes multi-tailles (Qt)
│   ├── app-64.png
│   ├── app-128.png
│   ├── app-256.png
│   ├── app.ico             # Icône Windows (Explorateur)
│   └── app.svg             # Icône vectorielle (optionnel, Linux)
├── lang/                   # Fichiers de langue (txt)
├── windows/                # Build Windows (gitignoré)
└── linux/                  # Build Linux (gitignoré)
```

## Points clés

### 1. CMakeLists.txt unique

Un seul `CMakeLists.txt` fonctionne sur les deux plateformes :

```cmake
cmake_minimum_required(VERSION 3.16)
project(ApplicationVide VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

add_executable(ApplicationVide
    main.cpp
    MainWindow.cpp
    Project.cpp
    ComponentToolbox.cpp
    UpdateChecker.cpp
    LangueManager.cpp
    resources.qrc
)

target_link_libraries(ApplicationVide PRIVATE Qt6::Widgets Qt6::Network)
target_include_directories(ApplicationVide PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
```

> **Note** : `app.rc` (icône Windows PE) est pré-compilée avec `windres` à la configuration et ajoutée conditionnellement. Sur Linux, elle est ignorée car l'outil n'est pas trouvé ou la cible n'est pas PE.

### 2. Compilation

**Windows (MinGW)** :
```bash
cmake -S . -B windows -G "MinGW Makefiles" \
  -DCMAKE_PREFIX_PATH=C:/Qt/6.x.x/mingw_64 \
  -DCMAKE_CXX_COMPILER=C:/Qt/Tools/mingwXXX_64/bin/g++.exe \
  -DCMAKE_MAKE_PROGRAM=C:/Qt/Tools/mingwXXX_64/bin/mingw32-make.exe
cmake --build windows
```

**Windows (MSVC)** :
```bash
cmake -S . -B windows -G "Visual Studio 17 2022" \
  -DCMAKE_PREFIX_PATH=C:/Qt/6.x.x/msvc2022_64
cmake --build windows --config Release
```

**Linux** :
```bash
cmake -S . -B linux -DCMAKE_PREFIX_PATH=/opt/Qt/6.x.x/gcc_64
cmake --build linux
```

### 3. Déploiement

- **Windows** : `windeployqt windows/ApplicationVide.exe` copie les DLLs Qt nécessaires
- **Linux** : `linuxdeployqt` ou打包 avec AppImage ; les bibliothèques système Qt sont souvent déjà installées

### 4. Pièges à éviter

- **Chemins** : utiliser `QDir::separator()` ou `/` (Qt le normalise) — jamais `\` en dur
- **Fichier `.rc`** : ne pas l'inclure dans `add_executable` sur Linux ; utiliser une condition ou une pré-compilation séparée
- **Utils système** : préférer `QProcess` et `QStandardPaths` aux appels système directs
- **Police** : tester avec une police de secours, Linux n'a pas Segoe UI
- **SSL** : `Qt6::Network` utilise OpenSSL ou Schannel selon la plateforme ; lier dynamiquement
