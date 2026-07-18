# Avoir une icône dans une application Qt multi-plateforme

## Contexte

Ce document explique comment un LLM doit configurer les icônes d'une application **Qt6 C++** pour qu'elles soient visibles à **tous les endroits** où un utilisateur peut les voir, sur **Windows et Linux**.

Une application Qt doit gérer **deux systèmes d'icônes distincts** :

| Usage | Windows | Linux |
|---|---|---|
| Barre de titre de la fenêtre | Qt resources (PNG via QRC) | Qt resources (PNG via QRC) |
| Barre des tâches | Qt resources + `.ico` ressource PE | Qt resources + `.desktop` |
| Alt+Tab / mission control | Qt resources (PNG via QRC) | Qt resources (PNG via QRC) |
| Explorateur de fichiers / Nautilus | Ressource `.ico` Windows (format PE) | Fichier `.desktop` pointant vers le PNG |
| Miniature dans l'Explorateur | Ressource `.ico` Windows | Icône du fichier `.desktop` |
| Icône du fichier `.exe` lui-même | Ressource `.ico` Windows intégrée | N/A (le binaire est ELF, pas d'icône embarquée) |

Un LLM qui ne fait que la moitié (ex. uniquement le QRC) produira une application où l'icône est invisible dans l'Explorateur Windows.

---

## 1. Création des fichiers d'icônes source

### 1.1. Créer les PNG multi-résolution

Qt a besoin de PNG (ou SVG) pour l'icône de la fenêtre.
Windows a besoin d'un `.ico` pour l'Explorateur.

**Prérequis** : créez 4 tailles de PNG (32, 64, 128, 256 px) dans un dossier `ico/`.

Méthode universelle (PowerShell avec .NET, Python avec Pillow, ou n'importe quel outil) :

```powershell
Add-Type -AssemblyName System.Drawing

$sizes = @(32, 64, 128, 256)
foreach ($s in $sizes) {
    $bmp = New-Object System.Drawing.Bitmap($s, $s)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality

    # Fond transparent
    $g.Clear([System.Drawing.Color]::Transparent)

    # DESSINER L'ICÔNE ICI (cercle bleu avec lettre "V" dans l'exemple)
    $brush = New-Object System.Drawing.Drawing2D.LinearGradientBrush(
        (New-Object System.Drawing.Point(0,0)),
        (New-Object System.Drawing.Point($s,$s)),
        [System.Drawing.Color]::FromArgb(52, 152, 219),
        [System.Drawing.Color]::FromArgb(41, 128, 185)
    )
    $g.FillEllipse($brush, 2, 2, $s-4, $s-4)

    $fontSize = [Math]::Max(10, $s/3)
    $font = New-Object System.Drawing.Font("Segoe UI", $fontSize, [System.Drawing.FontStyle]::Bold)
    $fmt = New-Object System.Drawing.StringFormat
    $fmt.Alignment = [System.Drawing.StringAlignment]::Center
    $fmt.LineAlignment = [System.Drawing.StringAlignment]::Center
    $g.DrawString("V", $font, [System.Drawing.Brushes]::White,
        [System.Drawing.RectangleF]::new(0, 0, $s, $s), $fmt)

    $bmp.Save("ico\app-$s.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose()
    $bmp.Dispose()
}
```

**Points importants :**
- Utilisez `Format32bppArgb` (transparence) — Qt gère la transparence dans les icônes
- Le fond doit être **transparent** pour les icônes de fenêtre, pas une couleur unie

### 1.2. Créer le fichier `.ico` Windows

Le format `.ico` est un conteneur qui peut stocker plusieurs résolutions.
Windows lit la meilleure résolution disponible pour chaque contexte d'affichage.

```powershell
Add-Type -AssemblyName System.Drawing

$sizes = @(32, 64)  # 32px pour l'Explorateur en mode liste, 64px pour les grandes icônes
$images = @()
foreach ($s in $sizes) {
    $images += [System.Drawing.Bitmap]::FromFile("ico\app-$s.png")
}

$stream = [System.IO.File]::Open("ico\app.ico", [System.IO.FileMode]::Create)
$sw = New-Object System.IO.BinaryWriter($stream)

# En-tête ICO
$sw.Write([UInt16]0)                    # reserved (must be 0)
$sw.Write([UInt16]1)                    # type: 1 = ICO, 2 = CUR
$sw.Write([UInt16]$images.Count)        # number of images

$offset = 6 + $images.Count * 16        # debut des données image

# Répertoire des images (16 bytes chacune)
for ($i = 0; $i -lt $images.Count; $i++) {
    $img = $images[$i]
    $ms = New-Object System.IO.MemoryStream
    $img.Save($ms, [System.Drawing.Imaging.ImageFormat]::Png)
    $data = $ms.ToArray()

    $w = if ($img.Width -ge 256) { 0 } else { $img.Width }
    $h = if ($img.Height -ge 256) { 0 } else { $img.Height }

    $sw.Write([Byte]$w)                 # largeur (0 = 256)
    $sw.Write([Byte]$h)                 # hauteur (0 = 256)
    $sw.Write([Byte]0)                  # couleurs palette (0 = pas de palette)
    $sw.Write([Byte]0)                  # reserved
    $sw.Write([UInt16]1)                # planes (must be 0 or 1)
    $sw.Write([UInt16]32)               # bits par pixel
    $sw.Write([UInt32]$data.Length)     # taille des données
    $sw.Write([UInt32]$offset)          # offset dans le fichier

    $offset += $data.Length
    $ms.Dispose()
}

# Données des images (PNG bruts)
for ($i = 0; $i -lt $images.Count; $i++) {
    $ms = New-Object System.IO.MemoryStream
    $images[$i].Save($ms, [System.Drawing.Imaging.ImageFormat]::Png)
    $data = $ms.ToArray()
    $sw.Write($data)
    $ms.Dispose()
    $images[$i].Dispose()
}

$sw.Close(); $stream.Close()
```

**Variante Python :**
```python
from PIL import Image
import struct

sizes = [32, 64]
images = [Image.open(f"ico/app-{s}.png") for s in sizes]

with open("ico/app.ico", "wb") as f:
    # Header
    f.write(struct.pack("<HHH", 0, 1, len(images)))
    offset = 6 + len(images) * 16
    data_list = []
    for img in images:
        from io import BytesIO
        buf = BytesIO()
        img.save(buf, "PNG")
        data = buf.getvalue()
        data_list.append(data)
        w = 0 if img.width >= 256 else img.width
        h = 0 if img.height >= 256 else img.height
        f.write(struct.pack("<BBBBHHII", w, h, 0, 0, 1, 32, len(data), offset))
        offset += len(data)
    for data in data_list:
        f.write(data)
```

**Structure du fichier ICO :**
```
[HEADER]    6 bytes   reserved(2) + type(2) + count(2)
[DIR]       16×N      pour chaque image : w, h, palette, reserved, planes, bpp, size, offset
[DATA]      variable  données PNG de chaque image
```

### 1.3. Créer un SVG (optionnel, Linux-friendly)

Le SVG est un format vectoriel en XML, lisible par n'importe quel éditeur de texte.
Certains gestionnaires de fichiers Linux le préfèrent au PNG.

```xml
<!-- ico/app.svg -->
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 256 256">
  <defs>
    <linearGradient id="g" x1="0" y1="0" x2="256" y2="256">
      <stop offset="0" stop-color="#3498db"/>
      <stop offset="1" stop-color="#2980b9"/>
    </linearGradient>
  </defs>
  <circle cx="128" cy="128" r="120" fill="url(#g)" stroke="#1a5276" stroke-width="6"/>
  <text x="128" y="164" font-family="Arial,sans-serif" font-size="140" font-weight="bold"
        fill="white" text-anchor="middle">V</text>
</svg>
```

---

## 2. Intégration Qt (icône dans l'application)

### 2.1. Fichier `resources.qrc`

Le fichier `.qrc` (Qt Resource Collection) déclare les fichiers embarqués dans l'exécutable.

```xml
<RCC>
    <qresource prefix="/">
        <file>ico/app-32.png</file>
        <file>ico/app-64.png</file>
        <file>ico/app-128.png</file>
        <file>ico/app-256.png</file>
    </qresource>
</RCC>
```

**Règles :**
- Le `prefix` est la racine virtuelle — les fichiers sont accessibles via `:/ico/app-256.png`
- Les chemins sont **relatifs au fichier `.qrc`** lui-même
- Les séparateurs sont des `/` (même sur Windows)
- On peut avoir plusieurs `<qresource>` avec des préfixes différents

### 2.2. CMakeLists.txt — Configuration obligatoire

```cmake
cmake_minimum_required(VERSION 3.16)
project(ApplicationVide VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ACTIVER CES DEUX LIGNES :
set(CMAKE_AUTOMOC ON)   # Traitement automatique des fichiers Qt MOC
set(CMAKE_AUTORCC ON)   # <-- COMPILATION AUTOMATIQUE DES .qrc

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(ApplicationVide
    main.cpp
    MainWindow.cpp
    resources.qrc        # CMake compile automatiquement ce fichier
)

target_link_libraries(ApplicationVide PRIVATE Qt6::Widgets)
```

**⚠️ Piège :** `CMAKE_AUTORCC` doit être `ON`. Sans ça, le fichier `.qrc` est listé dans les sources mais jamais compilé. Les PNG ne seront pas embarqués et l'icône sera invisible.

**Vérification** : regardez la sortie de cmake --build. Vous devez voir :
```
[ 20%] Automatic RCC for resources.qrc
[ 30%] Building CXX object .../qrc_resources.cpp.obj
```

### 2.3. main.cpp — Chargement de l'icône

```cpp
#include <QApplication>
#include <QIcon>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("ApplicationVide");

    // Charger plusieurs tailles pour que Qt choisisse la meilleure
    QIcon icone;
    icone.addFile(":/ico/app-256.png", QSize(256, 256));
    icone.addFile(":/ico/app-128.png", QSize(128, 128));
    icone.addFile(":/ico/app-64.png",  QSize(64, 64));
    icone.addFile(":/ico/app-32.png",  QSize(32, 32));

    // Définir l'icône pour toute l'application
    app.setWindowIcon(icone);

    MainWindow fenetre;
    fenetre.setWindowIcon(icone);  // Surcharge si besoin d'une icône différente
    fenetre.show();

    return app.exec();
}
```

**Détail important :** `QIcon::addFile()` ne remplace pas les icônes précédentes — il les **ajoute**. Qt choisira la meilleure taille disponible selon le contexte d'affichage. Ajoutez toujours plusieurs résolutions.

### 2.4. Vérification que les PNG sont bien embarqués

Les fichiers du QRC sont compilés dans l'exécutable. On peut vérifier leur présence :

```bash
# Rechercher l'en-tête PNG dans le binaire
# (hex: 89 50 4E 47 = \x89PNG)
```

```powershell
$path = "windows\ApplicationVide.exe"
$bytes = [System.IO.File]::ReadAllBytes($path)
for ($i = 0; $i -lt $bytes.Length - 4; $i++) {
    if ($bytes[$i] -eq 0x89 -and $bytes[$i+1] -eq 0x50 -and
        $bytes[$i+2] -eq 0x4E -and $bytes[$i+3] -eq 0x47) {
        Write-Output "PNG trouvé à l'offset $i"
        break
    }
}
```

---

## 3. Icône dans l'Explorateur Windows (ressource .ico)

Windows ne lit **pas** les PNG du QRC pour afficher l'icône dans l'Explorateur.
Il lit la **ressource ICON** du format PE (Portable Executable) — un système complètement indépendant de Qt.

### 3.1. Fichier de ressource Windows : `app.rc`

Le fichier `.rc` (Resource Definition) est compilé par `windres` (MinGW) ou `rc.exe` (MSVC).

```
IDI_ICON1 ICON "ico/app.ico"
```

- `IDI_ICON1` est l'identifiant de la ressource (peut être n'importe quel nom ou nombre)
- `ICON` est le type de ressource
- `"ico/app.ico"` est le chemin relatif vers le fichier `.ico`

Pour MSVC, la syntaxe est identique mais le compilateur est `rc.exe`.

### 3.2. Compilation avec `windres` (MinGW)

`windres` est l'équivalent de `rc.exe` pour MinGW/GCC :

```bash
windres -i app.rc -o app_icon.o
```

**Problème connu avec les chemins exotiques :**

Sur certains systèmes, les chemins contenant des **crochets `[]`** ou un **dièse `#`** (ex: `[WorkSpace]`, `#iA`) peuvent causer des erreurs avec `windres` appelé via CMake :
```
cc1.exe: fatal error: ...\app.rc\: Invalid argument
```

**Solution :** ne pas passer par le mécanisme CMake RC, mais pré-compiler le `.rc` en `.o` manuellement au moment de la configuration (phase `configure` de CMake), puis ajouter le `.o` aux sources de l'exécutable.

### 3.3. Intégration complète dans `CMakeLists.txt`

```cmake
# 1. Copier le .ico dans le répertoire de build (pour windres)
file(COPY ico/app.ico DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ico)

# 2. Trouver windres (MinGW) ou rc.exe (MSVC)
if(WIN32)
    if(MINGW)
        # MinGW : windres se trouve dans le dossier Tools de Qt
        set(WINDRES "C:/Qt/Tools/mingwXXX_64/bin/windres.exe")
    elseif(MSVC)
        # MSVC : utiliser rc.exe (dans le PATH du terminal de développement)
        set(WINDRES "rc.exe")
    endif()
endif()

# 3. Pré-compiler le .rc en .o pour éviter les problèmes de chemin
if(WIN32 AND MINGW)
    set(RC_OUT "${CMAKE_CURRENT_BINARY_DIR}/app_icon.o")
    if(NOT EXISTS "${RC_OUT}")
        execute_process(
            COMMAND ${WINDRES} -i "${CMAKE_CURRENT_SOURCE_DIR}/app.rc" -o "${RC_OUT}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        )
    endif()

    # Ajouter le .o compilé aux sources de l'exécutable
    add_executable(ApplicationVide
        main.cpp
        MainWindow.cpp
        resources.qrc
        ${RC_OUT}          # <-- ressource Windows embarquée
    )
else()
    add_executable(ApplicationVide
        main.cpp
        MainWindow.cpp
        resources.qrc
    )
endif()
```

**Important :** `execute_process` s'exécute à la configuration (quand vous lancez `cmake -B build`),
pas à la compilation. Si vous modifiez `app.rc` ou `app.ico`, il faut reconfigurer.

**Solution alternative** pour éviter de reconfigurer : utiliser `add_custom_command` avec `BYPRODUCTS` :

```cmake
set(RC_OUT "${CMAKE_CURRENT_BINARY_DIR}/app_icon.o")
add_custom_command(
    OUTPUT ${RC_OUT}
    COMMAND ${WINDRES} -i "${CMAKE_CURRENT_SOURCE_DIR}/app.rc" -o "${RC_OUT}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/app.rc" "${CMAKE_CURRENT_SOURCE_DIR}/ico/app.ico"
)
```

Cette approche fonctionne si `windres` est accessible dans le PATH utilisé par CMake pendant le build.

### 3.4. Vérification que l'icône .ico est embarquée

```powershell
$path = "windows\ApplicationVide.exe"
$bytes = [System.IO.File]::ReadAllBytes($path)
for ($i = 0; $i -lt $bytes.Length - 4; $i++) {
    if ($bytes[$i] -eq 0x00 -and $bytes[$i+1] -eq 0x00 -and
        $bytes[$i+2] -eq 0x01 -and $bytes[$i+3] -eq 0x00) {
        Write-Output "Ressource ICO trouvée à l'offset $i"
        break
    }
}
```

```bash
# Ou avec un outil externe comme Resource Hacker
```

---

## 4. Icône sur Linux (fichier .desktop)

Linux n'embarque **pas** d'icône dans le binaire ELF. L'icône est déclarée dans un fichier `.desktop`.

### 4.1. Créer le fichier `.desktop`

```ini
[Desktop Entry]
Type=Application
Name=ApplicationVide
Comment=Application Qt6 de base
Icon=/opt/ApplicationVide/ico/app-256.png   # Chemin ABSOLU vers le PNG
Exec=/opt/ApplicationVide/ApplicationVide    # Chemin ABSOLU vers le binaire
Terminal=false
Categories=Development;
```

### 4.2. Où placer le fichier `.desktop`

| Emplacement | Portée |
|---|---|
| `~/.local/share/applications/` | Utilisateur uniquement |
| `/usr/share/applications/` | Tous les utilisateurs (root requis) |
| `/usr/local/share/applications/` | Applications locales (root requis) |

```bash
# Installer pour l'utilisateur courant
cp ApplicationVide.desktop ~/.local/share/applications/

# Mettre à jour le cache (certains environnements)
update-desktop-database ~/.local/share/applications/
```

### 4.3. Utiliser un SVG pour l'icône Linux

Les environnements de bureau Linux modernes (GNOME, KDE) supportent les SVG pour les icônes.

```ini
Icon=/opt/ApplicationVide/ico/app.svg
```

### 4.4. Alternative : installer dans le thème d'icônes système

```bash
# Copier dans le thème hicolor (standard Freedesktop)
sudo mkdir -p /usr/share/icons/hicolor/256x256/apps/
sudo cp ico/app-256.png /usr/share/icons/hicolor/256x256/apps/ApplicationVide.png

# Ou utiliser un SVG
sudo cp ico/app.svg /usr/share/icons/hicolor/scalable/apps/ApplicationVide.svg

# Mettre à jour le cache
sudo gtk-update-icon-cache /usr/share/icons/hicolor/
```

Dans le `.desktop` :
```ini
Icon=ApplicationVide   # Sans chemin ni extension, cherche dans les thèmes systèmes
```

### 4.5. Intégration dans CMakeLists.txt (installation)

```cmake
# Installation Linux
install(TARGETS ApplicationVide DESTINATION bin)
install(FILES ico/app.svg DESTINATION share/icons/hicolor/scalable/apps RENAME ApplicationVide.svg)
install(FILES ApplicationVide.desktop DESTINATION share/applications)
```

```bash
# Build et install
cmake --build linux
cmake --install linux --prefix /usr/local
```

---

## 5. Déploiement

### 5.1. Windows — `windeployqt`

`windeployqt` copie les DLLs Qt et les **plugins** nécessaires à côté de l'exécutable.
Sans cette étape, même les icônes du QRC peuvent ne pas s'afficher si les plugins de décodage PNG sont absents.

```bash
windeployqt windows/ApplicationVide.exe
```

**Ce que windeployqt copie :**
- Les DLLs Qt6 (Core, Gui, Widgets, etc.)
- Les plugins de **formats d'image** (`imageformats/qjpeg.dll`, etc.)
- Les plugins de **plateforme** (`platforms/qwindows.dll`)
- Les plugins de **styles** (`styles/qmodernwindowsstyle.dll`)
- Les plugins de **moteurs d'icônes** (`iconengines/qsvgicon.dll`)
- Les compilateurs redistribuables (libgcc, libstdc++, libwinpthread)

**Format d'image et icône :**
Qt6 intègre le décodage PNG directement dans QtGui — pas besoin de plugin spécifique pour les PNG.
En revanche, si vous utilisez des SVG, il faut le module Qt SVG et le plugin `qsvgicon.dll`.

### 5.2. Linux — déploiement manuel

```bash
# Copier le binaire
cp linux/ApplicationVide /usr/local/bin/

# Copier les icônes
cp -r ico /usr/local/share/ApplicationVide/

# Copier le fichier desktop
cp ApplicationVide.desktop /usr/local/share/applications/
```

Ou utiliser `ldd` pour trouver les dépendances Qt :
```bash
ldd linux/ApplicationVide | grep Qt
# Copier les librairies Qt manquantes dans le même dossier
```

### 5.3. Créer un installeur (Windows)

Pour une distribution professionnelle, créez un installeur avec **NSIS**, **Inno Setup** ou **WiX Toolset**.

Exemple minimal Inno Setup :
```iss
[Setup]
AppName=ApplicationVide
AppVersion=1.0
DefaultDirName={pf}\ApplicationVide

[Files]
Source: "windows\*"; DestDir: "{app}"; Flags: recursesubdirs

[Icons]
Name: "{group}\ApplicationVide"; Filename: "{app}\ApplicationVide.exe"
Name: "{commondesktop}\ApplicationVide"; Filename: "{app}\ApplicationVide.exe"
```

---

## 6. Centralisation dans `AppConfig.hpp`

Toutes les constantes du projet sont regroupées dans `AppConfig.hpp` :

```cpp
// AppConfig.hpp
#define APP_NAME "ApplicationVide"
#define APP_VERSION "1.0.0"
// Dossier icônes : "ico/"
// Formats supportés : PNG (32–256px), ICO (32+64px), SVG (optionnel)
// Préfixe Qt resource : ":/ico/"
```

C'est le fichier de référence pour les versions, URLs et formats d'icônes.

## 7. Arbre complet des fichiers

```
ApplicationVide/                          # Racine du projet
├── CMakeLists.txt                        # CMake avec AUTORCC ON
├── AppConfig.hpp                         # Config centralisée (version, URLs, icônes)
├── main.cpp                              # QIcon::addFile + setWindowIcon
├── MainWindow.hpp / MainWindow.cpp       # Fenêtre principale
├── UpdateChecker.hpp / UpdateChecker.cpp # Vérification de mise à jour
├── resources.qrc                         # Déclare les PNG pour Qt
├── app.rc                                # Déclare le .ico pour Windows PE
│
├── ico/                                  # Toutes les icônes
│   ├── app-32.png                        # PNG 32px (Qt)
│   ├── app-64.png                        # PNG 64px (Qt, icône par défaut)
│   ├── app-128.png                       # PNG 128px (Qt, barre des tâches)
│   ├── app-256.png                       # PNG 256px (Qt, haute résolution)
│   ├── app.ico                           # ICO Windows (32+64px)
│   └── app.svg                           # SVG vectoriel (optionnel, Linux)
│
├── windows/                              # Build Windows (sortie)
│   ├── ApplicationVide.exe               # Exécutable (PNG + ICO embarqués)
│   ├── Qt6Core.dll                       # Déployé par windeployqt
│   ├── Qt6Gui.dll
│   ├── Qt6Widgets.dll
│   ├── platforms/qwindows.dll            # Plugin de plateforme
│   └── imageformats/qjpeg.dll            # Plugin de décodage image
│
├── linux/                                # Build Linux (sortie)
│   ├── ApplicationVide                   # Exécutable ELF (PNG embarqués)
│   └── ...
│
├── ApplicationVide.desktop               # Lanceur Linux (optionnel)
│
├── comment_avoir_une_icone.md            # Ce fichier
└── AGENTS.md                             # Guide pour l'IA
```

---

## 7. Résumé des commandes complètes

### Windows

```bash
# 1. Générer les icônes (PowerShell)
powershell -File generate_icons.ps1

# 2. Configurer le projet
cmake -S . -B windows -G "MinGW Makefiles" ^
    -DCMAKE_PREFIX_PATH=C:/Qt/6.x.x/mingw_64 ^
    -DCMAKE_CXX_COMPILER=C:/Qt/Tools/mingwXXX_64/bin/g++.exe ^
    -DCMAKE_MAKE_PROGRAM=C:/Qt/Tools/mingwXXX_64/bin/mingw32-make.exe

# 3. Compiler
cmake --build windows

# 4. Vérifier les ressources embarquées
#    - Chercher "PNG" (89 50 4E 47) dans le .exe → ressource Qt OK
#    - Chercher "ICO" (00 00 01 00) dans le .exe → ressource Windows OK

# 5. Déployer les DLLs Qt
windeployqt windows/ApplicationVide.exe

# 6. Tester
windows/ApplicationVide.exe
```

### Linux (non testé, à adapter)

```bash
# 1. Générer les icônes
#    (créer les PNG et SVG manuellement ou avec ImageMagick)

# 2. Configurer
cmake -S . -B linux -DCMAKE_PREFIX_PATH=/opt/Qt/6.x.x/gcc_64

# 3. Compiler
cmake --build linux

# 4. Installer
sudo cmake --install linux --prefix /usr/local

# 5. Tester
/usr/local/bin/ApplicationVide
```

---

## 9. Pièges et solutions

### 9.1. Tableau récapitulatif

| Problème | Cause | Solution |
|---|---|---|
| Icône absente dans l'appli Qt (barre de titre, tâche) | `CMAKE_AUTORCC` désactivé | Ajouter `set(CMAKE_AUTORCC ON)` dans CMakeLists.txt |
| Icône absente dans l'appli Qt | PNG non trouvé dans les ressources | Vérifier le chemin dans `resources.qrc` et `main.cpp` |
| Icône absente dans l'Explorateur Windows | Pas de ressource `.ico` dans le .exe | Ajouter `app.rc` + `windres` + linker le `.o` |
| `windres` échoue : `cc1.exe: \: No such file` | Chemin avec `[]` ou `#` | Pré-compiler `.rc` via `execute_process` au configure |
| `windres` échoue : `unrecognized command-line option` | `CMAKE_RC_FLAGS` transmis au mauvais endroit | Ne pas utiliser `CMAKE_RC_FLAGS`, compiler manuellement |
| Icône absente après avoir tout fait | `windeployqt` pas exécuté | Les plugins de décodage d'image manquent |
| Erreur DLL au lancement sur Windows | Mauvaises DLL Qt dans le PATH | Lancer depuis le dossier déployé ou exécuter `windeployqt` |
| Icône absente sur Linux | Pas de fichier `.desktop` | Créer et installer un fichier `.desktop` |
| Icône absente sur Linux | Chemin d'icône relatif dans `.desktop` | Utiliser un chemin **absolu** vers le PNG/SVG |
| L'icône apparaît déformée | Mauvaise résolution ou format | Utiliser des PNG avec transparence (pas de fond coloré uni) |
| L'icône ne s'affiche pas sous Wine | Wine gère mal certaines ressources PE | Tester sur Windows natif |

### 9.2. Détection de l'environnement dans CMake

```cmake
if(WIN32)
    message(STATUS "Plateforme : Windows")
    if(MINGW)
        message(STATUS "Compilateur : MinGW")
    elseif(MSVC)
        message(STATUS "Compilateur : MSVC")
    endif()
elseif(UNIX AND NOT APPLE)
    message(STATUS "Plateforme : Linux")
elseif(APPLE)
    message(STATUS "Plateforme : macOS (non supporté actuellement)")
endif()
```

### 9.3. Test de l'icône dans l'Explorateur

Pour forcer Windows à rafraîchir le cache d'icônes après avoir changé l'icône de l'exécutable :

```bat
:: Option 1 : Redémarrer l'Explorateur
taskkill /f /im explorer.exe
start explorer.exe

:: Option 2 : Vider le cache d'icônes (Windows 10+)
del /a "%LocalAppData%\IconCache.db"
```

Si l'icône n'apparaît toujours pas dans l'Explorateur :
1. Vérifiez que le `.ico` est bien dans le `.exe` (cherchez `00 00 01 00` dans le binaire)
2. Vérifiez que l'`.ico` contient au moins une résolution 32×32
3. Déplacez/copiez le `.exe` vers un autre dossier — Windows met en cache par emplacement

### 9.4. Cas particulier : icône dans la barre des tâches

Sur Windows 10/11, la barre des tâches peut ne pas afficher l'icône si :

1. **L'application n'est pas épinglée** : Windows utilise l'icône de la fenêtre (Qt resources)
2. **L'application est épinglée** : Windows utilise le cache de la cible épinglée, pas l'exécutable actuel
3. **Solution** : Désépingler et réépingler, ou modifier le `AppUserModelID`

---

## 10. Code complet prêt à l'emploi

### AppConfig.hpp

```cpp
#ifndef APPCONFIG_HPP
#define APPCONFIG_HPP

#define APP_NAME "ApplicationVide"
#define APP_VERSION "1.0.0"
#define UPDATE_CHECK_URL "https://raw.githubusercontent.com/.../main/version.json"
#define APP_HOMEPAGE_URL "https://github.com/..."

// Icônes : dossier "ico/" — formats PNG (32-256px), ICO, SVG
#endif
```

### CMakeLists.txt complet

```cmake
cmake_minimum_required(VERSION 3.16)
project(ApplicationVide VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

file(COPY ico/app.ico DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ico)

if(WIN32 AND MINGW)
    set(WINDRES "C:/Qt/Tools/mingw1310_64/bin/windres.exe")
    set(RC_OUT "${CMAKE_CURRENT_BINARY_DIR}/app_icon.o")
    if(NOT EXISTS "${RC_OUT}")
        execute_process(
            COMMAND ${WINDRES} -i "${CMAKE_CURRENT_SOURCE_DIR}/app.rc" -o "${RC_OUT}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        )
    endif()
    add_executable(ApplicationVide main.cpp resources.qrc ${RC_OUT})
else()
    add_executable(ApplicationVide main.cpp resources.qrc)
endif()

target_link_libraries(ApplicationVide PRIVATE Qt6::Widgets)
target_include_directories(ApplicationVide PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
```

### resources.qrc

```xml
<RCC>
    <qresource prefix="/">
        <file>ico/app-32.png</file>
        <file>ico/app-64.png</file>
        <file>ico/app-128.png</file>
        <file>ico/app-256.png</file>
    </qresource>
</RCC>
```

### app.rc

```
IDI_ICON1 ICON "ico/app.ico"
```

### main.cpp

```cpp
#include <QApplication>
#include <QIcon>
#include "AppConfig.hpp"
#include "MainWindow.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral(APP_NAME));

    QIcon icone;
    icone.addFile(":/ico/app-256.png", QSize(256, 256));
    icone.addFile(":/ico/app-128.png", QSize(128, 128));
    icone.addFile(":/ico/app-64.png",  QSize(64, 64));
    icone.addFile(":/ico/app-32.png",  QSize(32, 32));
    app.setWindowIcon(icone);

    MainWindow fenetre;
    fenetre.setWindowIcon(icone);
    fenetre.show();

    return app.exec();
}
```

---

## 11. Pour les LLM : checklist à suivre

Quand vous devez ajouter une icône à un projet Qt, appliquez cette checklist **dans l'ordre** :

1. [ ] Créer les PNG en 32, 64, 128, 256 px dans `ico/`
2. [ ] Créer le `.ico` Windows (contient 32px + 64px)
3. [ ] Créer `resources.qrc` listant les PNG
4. [ ] Ajouter `set(CMAKE_AUTORCC ON)` dans `CMakeLists.txt` — **ne pas oublier**
5. [ ] Ajouter `resources.qrc` aux sources de `add_executable`
6. [ ] Ajouter `QIcon::addFile` + `setWindowIcon` dans `main.cpp`
7. [ ] Créer `app.rc` pointant vers `ico/app.ico`
8. [ ] Compiler `app.rc` en `.o` avec `windres` (via `execute_process` dans CMake)
9. [ ] Ajouter le `.o` aux sources de `add_executable`
10. [ ] Compiler et vérifier que PNG et ICO sont dans le binaire
11. [ ] Lancer `windeployqt` pour déployer les DLLs et plugins
12. [ ] Vérifier/remplir `AppConfig.hpp` avec la version, les URLs et les infos icônes
13. [ ] Sur Linux : créer un fichier `.desktop` avec le chemin absolu vers le PNG
