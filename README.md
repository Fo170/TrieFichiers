# ApplicationVide

Application Qt6 vide — code source unique compilable sous Windows et Linux.

Testé et fonctionnel sous **Windows 10/11** (MinGW).

## Structure

```
ApplicationVide/
├── CMakeLists.txt           # Build système (Qt6 Widgets + Network)
├── AppConfig.hpp            # Config centralisée (version, URLs, icônes)
├── main.cpp                 # Point d'entrée
├── MainWindow.hpp/.cpp      # Fenêtre principale (menus, toolbar, dock)
├── UpdateChecker.hpp/.cpp   # Vérification de mise à jour (HTTP)
├── Project.hpp/.cpp         # Projet JSON (load/save)
├── ComponentToolbox.hpp/.cpp# Boîte à outils de composants
├── resources.qrc            # Ressources (icônes)
├── version.json             # Version distante pour mise à jour
├── app.rc                   # Ressource Windows (.ico pour Explorateur)
├── ico/                     # Icônes PNG/ICO/SVG
├── windows/                 # Build Windows (exécutable + DLLs)
├── linux/                   # Build Linux (sortie)
├── guide_implantation_fonctionnalites.md  # Guide complet : 6 fonctionnalités Qt + pièges
├── "Qt GUI — bibliothèques et technologies utilisées.md"  # Dépendances Qt détaillées
└── AGENTS.md                # Guide de travail pour LLM (OpenCode)
```

## Prérequis

- **Qt 6** (modules Widgets, Network)
- **CMake** ≥ 3.16
- Compilateur C++17

## Compilation

### Windows (MinGW) — testé

```bash
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

## Déploiement

### Windows

```bash
windeployqt windows/ApplicationVide.exe
```

### Linux

```bash
# Copier les librairies Qt manuellement (ou lier statiquement)
```

## Fonctionnalités

- Menu **Fichier** → 📂 Charger / 💾 Sauvegarder projet JSON / ❌ Quitter
- Barre d'outils → 📂 Charger / 💾 Sauvegarder
- Menu **Outils** → 🧰 Boîte à outils (composants sélectionnables)
- Menu **?** → 🔄 Vérifier les mises à jour / ℹ️ À propos
- Menu **Langue** → bascule multilingue (🇫🇷 Français, 🇬🇧 English)
- Détection automatique de la langue du système (Windows et Linux)
- Projet au format JSON (version, name, components[])
- Détection de mise à jour en ligne via `version.json` (GitHub raw)
- **Auto-réparation** : fichiers de langue téléchargés depuis GitHub s'ils sont manquants
- **Auto-régénération** : fichier `application.ini` recréé immédiatement s'il est supprimé
- Configuration centralisée dans `AppConfig.hpp` (version, URLs, icônes)

## Documentation

| Document | Contenu |
|---|---|
| `guide_implantation_fonctionnalites.md` | Code complet des 6 fonctionnalités Qt + 20 sections de pièges et bonnes pratiques |
| `Qt GUI — bibliothèques et technologies utilisées.md` | Liste exhaustive des classes Qt par catégorie |
| `langues.md` | Ajouter / télécharger une langue |
| `comment_avoir_une_icone.md` | Icônes multi-plateforme |
| `verification_mise_a_jour.md` | Fonctionnement de la mise à jour |
| `AGENTS.md` | Guide de travail pour LLM (OpenCode)
