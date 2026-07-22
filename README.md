# TrieFichiers

Application Qt6 de nettoyage et tri de fichiers — multi-plateforme (Windows + Linux).

## Fonctionnalités

- **Nettoyage de dossiers** : analyse + suppression des fichiers vides, Thumbs.db, dossiers vides
- **Analyse préalable** : voir le nombre d'éléments avant de les supprimer
- **Multi-langue** : Français / Anglais, détection automatique
- **Vérification de mises à jour** en ligne via GitHub
- **Barre d'outils** avec outils de nettoyage
- **Déduplicateur d'extensions** : corrige les fichiers avec extensions dupliquées (`.torrent.torrent` → `.torrent`) — v1.0.3

## Structure

```
TrieFichiers/
├── CMakeLists.txt
├── main.cpp
├── AppConfig.hpp
├── MainWindow.hpp/.cpp
├── ComponentToolbox.hpp/.cpp
├── FolderCleaner.hpp/.cpp
├── CleanupDialog.hpp/.cpp
├── UpdateChecker.hpp/.cpp
├── LangueManager.hpp/.cpp
├── Project.hpp/.cpp
├── resources.qrc
├── version.json
├── app.rc
├── toolchain-mingw64.cmake
├── ico/
├── lang/
└── README.md
```

## Prérequis

- Qt 6.4+ (Widgets, Network)
- CMake ≥ 3.16
- Compilateur C++17

## Compilation

### Linux

```bash
cmake -S . -B linux -DCMAKE_PREFIX_PATH=/usr -DCMAKE_BUILD_TYPE=Release
cmake --build linux
```

### Windows (cross-compilation depuis Linux)

```bash
cmake -S . -B windows \
  -G "Unix Makefiles" \
  -DCMAKE_TOOLCHAIN_FILE=toolchain-mingw64.cmake \
  -DCMAKE_PREFIX_PATH=/home/administrateur/Qt/6.4.2/mingw_64 \
  -DQT_HOST_PATH=/usr \
  -DCMAKE_BUILD_TYPE=Release
cmake --build windows
```

### Windows (natif MinGW)

```bash
cmake -S . -B windows -G "MinGW Makefiles" ^
  -DCMAKE_PREFIX_PATH=C:/Qt/6.x.x/mingw_64
cmake --build windows
```

## Déploiement

```bash
windeployqt windows/TrieFichiers.exe
```

## Documentation

| Document | Contenu |
|---|---|
| `README.md` | Guide |

