# Qt GUI — bibliothèques et technologies utilisées

## Modules CMake

```
Qt6::Widgets    — UI complète (fenêtres, widgets, graphics, événements)
Qt6::Network    — Mise à jour, téléchargement langues
```

## Arborescence des classes Qt utilisées

| Famille | Classes |
|---------|---------|
| **Application/Fenêtres** | `QApplication`, `QMainWindow`, `QDialog`, `QWidget`, `QSplitter` |
| **Graphics/2D** | `QGraphicsScene`, `QGraphicsView`, `QGraphicsObject`, `QGraphicsLineItem`, `QGraphicsEllipseItem`, `QGraphicsPolygonItem`, `QGraphicsSimpleTextItem` |
| **Événements** | `QGraphicsSceneMouseEvent`, `QGraphicsSceneHoverEvent`, `QGraphicsSceneWheelEvent`, `QGraphicsSceneContextMenuEvent`, `QKeyEvent`, `QWheelEvent`, `QMouseEvent` |
| **Pinceaux/formes** | `QPainter`, `QPen`, `QFont`, `QPainterPath`, `QPainterPathStroker`, `QStyleOptionGraphicsItem`, `QFontMetrics` |
| **Widgets de formulaire** | `QPushButton`, `QLabel`, `QLineEdit`, `QComboBox`, `QCheckBox`, `QGroupBox`, `QSpinBox`, `QDoubleSpinBox`, `QTableWidget`, `QTabWidget`, `QTreeWidget` |
| **Mise en page** | `QVBoxLayout`, `QHBoxLayout`, `QFormLayout`, `QHeaderView`, `QFrame` |
| **Fichiers/IO** | `QFile`, `QFileInfo`, `QDir`, `QTextStream`, `QFileDialog` |
| **JSON** | `QJsonDocument`, `QJsonObject`, `QJsonArray` |
| **Réseau** | `QNetworkAccessManager`, `QNetworkReply`, `QNetworkRequest` |
| **Actions/menus** | `QMenu`, `QMenuBar`, `QAction`, `QToolBar`, `QStatusBar` |
| **Utilitaires** | `QTimer`, `QIcon`, `QMap`, `QVector`, `QVariantMap`, `QString`, `QStringList`, `QPointF`, `QInputDialog`, `QMessageBox`, `QCoreApplication`, `QSettings`, `QVersionNumber`, `QDesktopServices` |
| **Math** | `QtMath` (qFabs, qFloor, etc.) |

## Résumé de l'architecture Qt

- **Pas de Qt Quick/QML** — 100% widgets Qt classiques avec `QGraphicsScene`/`QGraphicsView` pour l'éditeur visuel et les aperçus monde/animal.
- **Rendu 2D custom** via `QPainter` sur des `QGraphicsObject` (neurones, synapses, organes).
- **Pas de Qt3D/OpenGL** — tout est en 2D logicielle.
- **CMAKE_AUTOMOC** activé (signaux/slots Qt présents dans `AnimalFenetre`, `MondeFenetre`, `MainWindow`, `GraphScene`).
- **CMAKE_AUTORCC** activé (ressources `.qrc`).
- **Ressources** embarquées via `resources.qrc` (icônes PNG dans `ico/`).
- **Polices** : configuration du fallback émoji dans `main.cpp`.

---

## Problème émoji (noir et blanc → couleur)

### Cause

Sous Windows, `QFont::setFamilies()` définit une liste de polices de secours. Le code original :

```cpp
QFont f = app.font();
QStringList fam = f.families();       // → ["Segoe UI", ...]
fam << "Noto Color Emoji" << "Symbola" << "Segoe UI Emoji";
f.setFamilies(fam);
app.setFont(f);
```

`Segoe UI` contient ses propres glyphes d'emoji en **noir et blanc**. Qt utilise la première police supportant le caractère → il prend le B&W de `Segoe UI` et ne descend jamais jusqu'à `Segoe UI Emoji`.

Sous Qt 6.3+ (DirectWrite au lieu de GDI), la chaîne explicite **écrase** la chaîne native de DirectWrite qui, elle, sait router automatiquement les emoji vers `Segoe UI Emoji`.

### Solution

Mettre les polices emoji **avant** `Segoe UI` :

```cpp
QFont f = app.font();
QStringList fam = f.families();
fam.removeAll("Segoe UI Emoji");
fam.removeAll("Noto Color Emoji");
fam.removeAll("Apple Color Emoji");
fam.removeAll("Symbola");
fam.push_front("Segoe UI Emoji");
fam.push_front("Noto Color Emoji");
fam.push_front("Apple Color Emoji");
fam.push_front("Symbola");
f.setFamilies(fam);
app.setFont(f);
```

Ordre final : `Segoe UI Emoji` → `Noto Color Emoji` → `Apple Color Emoji` → `Symbola` → polices système.

**Note** : `Segoe UI Emoji` contient les lettres/chiffres en glyphes texte standards, pas en emoji. Les caractères ASCII ne sont pas affectés.

### Barres de titre Windows

Les titres sont dessinés par le système (zone non-client), pas par Qt. Windows utilise `Segoe UI` (B&W) — les emoji restent en noir et blanc. Limitation Windows, pas Qt.

---

## Erreurs DLL récurrentes — conflit de versions Qt

### Symptômes

```
Le point d'entrée de procédure _Z9qBadAllocv est introuvable
dans la bibliothèque de liens dynamiques.

Le point d'entrée de procédure _Z21qRegisterResourceDataiPKhS0_S0_
est introuvable dans la bibliothèque de liens dynamiques.
```

Symboles exportés par `Qt6Core.dll` : `qBadAlloc()` et `qRegisterResourceData()`.

### Cause

L'exécutable compile avec une version Qt (ex: 6.11.0) mais charge les DLL d'une **version différente**. Les symboles exportés changent entre versions mineures.

| Scénario | Cause |
|----------|-------|
| Build réutilisé avec DLL d'ancienne installation | `windeployqt` non exécuté après rebuild |
| Projet partagé entre machines avec Qt versions différentes | DLL version A, binaire compilé avec version B |
| Mise à jour Qt sans rebuild complet | Cache CMake + DLL obsolètes |

### Solution

1. **Nettoyer et rebuild** :
   ```bash
   rm -rf qtgui/build/CMakeCache.txt qtgui/build/CMakeFiles
   rm -f qtgui/build/*.dll qtgui/build/*.exe
   cmake -S qtgui -B qtgui/build -G Ninja \
       -DCMAKE_PREFIX_PATH="C:/Qt/6.11.0/mingw_64" \
       -DCMAKE_CXX_COMPILER="C:/Qt/Tools/mingw1310_64/bin/g++.exe" \
       -DCMAKE_MAKE_PROGRAM="C:/Qt/Tools/Ninja/ninja.exe"
   cmake --build qtgui/build
   ```

2. **Déployer les DLL** :
   ```bash
   windeployqt qtgui/build/AppNeurone.exe
   ```

3. **Prévention** :
   - Toujours exécuter `windeployqt` après `cmake --build`
   - Ne pas mélanger des DLL de versions Qt différentes
   - Après une mise à jour de Qt, rebuild from scratch
   - Ne pas versionner les DLL dans git
