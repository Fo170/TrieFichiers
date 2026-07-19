# Qt GUI — bibliothèques et technologies utilisées

## Modules Qt liés

Depuis `CMakeLists.txt` (ligne 9 & 35) :

| Module | Rôle |
|---|---|
| `Qt6::Widgets` | GUI complète : fenêtres, widgets, menus, dialogues |
| `Qt6::Network` | Requêtes HTTP (mise à jour, téléchargement des langues) |

---

## Classes Qt par catégorie

### Fenêtrage & widgets de base (`<QWidgets>`)

- `QApplication` — point d'entrée de l'application
- `QMainWindow` — fenêtre principale avec menus, toolbar, statusbar, dock
- `QDockWidget` — panneau latéral (boîte à outils)
- `QWidget` — classe de base
- `QLabel` — texte central, titre de la boîte à outils
- `QMenuBar` / `QMenu` — barre de menus et sous-menus
- `QToolBar` — barre d'outils
- `QStatusBar` — barre d'état
- `QAction` / `QActionGroup` — actions cliquables (menu + toolbar)
- `QTreeWidget` / `QTreeWidgetItem` / `QHeaderView` — arbre de composants
- `QVBoxLayout` — agencement vertical
- `QKeySequence` — raccourcis clavier (Ctrl+O, Ctrl+S)

### Dialogues

- `QFileDialog` — boîte de dialogue ouvrir/sauvegarder un fichier
- `QMessageBox` — boîtes À propos, avertissement, mise à jour
- `QAbstractButton` — accès aux boutons de `QMessageBox`

### Événements

- `QCloseEvent` — interception de la fermeture (sauvegarde des réglages)

### Ressources & icônes

- `QIcon` — icône multi-taille de l'application/fenêtre
- `QSize` — tailles d'icône (16, 32, 64, 128, 256 px)
- **Qt Resource System** — fichier `resources.qrc` compile les PNG dans l'exécutable (préfixe `:/ico/`)
- `app.rc` → `windres` → icône `.ico` pour l'Explorateur Windows

### Réseau (`<QtNetwork>`)

- `QNetworkAccessManager` — client HTTP (vérification de version, téléchargement de langue)
- `QNetworkReply` — réponse HTTP
- `QNetworkRequest` — requête HTTP (avec User-Agent et timeout 10s)

### JSON (`<QtCore>`)

- `QJsonDocument` / `QJsonObject` / `QJsonArray` — sérialisation du projet
- `QJsonParseError` — gestion d'erreur de parsing

### Fichiers & locales

- `QFile` / `QFileInfo` — lecture/écriture de fichiers
- `QDir` — opérations sur les répertoires
- `QTextStream` — lecture UTF-8 des fichiers de langue (via `setEncoding(QStringConverter::Utf8)`)
- `QSettings` — lecture/écriture de `application.ini` (format INI)
- `QCoreApplication` — accès au chemin de l'exécutable
- `QLocale` — détection de la langue système

### Utilitaires (`<QtCore>`)

- `QObject` — classe de base Qt, parenté mémoire
- `QString` / `QStringList` — chaînes
- `QHash` — stockage des traductions (clé → valeur)
- `QVariantMap` — données d'un composant sélectionné
- `QVersionNumber` — comparaison de versions sémantiques (mise à jour)
- `QUrl` — construction d'URL
- `QDesktopServices::openUrl` — ouverture du navigateur pour le téléchargement

---

## Technologies Qt mises en œuvre

| Technologie | Utilisation |
|---|---|
| **Meta-Object System** (`Q_OBJECT`, signaux/slots) | Communication inter-composants, 6 signaux personnalisés |
| **Qt5-style connect** (functor syntax) | Connexions signaux/slots modernes (vérification à la compilation) |
| **Qt Resource System** (`.qrc`) | Embarquement des icônes PNG dans l'exécutable |
| **Parenting mémoire** (QObject parent) | Aucun `delete` manuel, tout est nettoyé automatiquement |
| **QSettings (INI)** | Persistance des réglages (langue, géométrie) |
| **QStringConverter** | Décodage UTF-8 des fichiers de langue |
| **QNetworkRequest::setTransferTimeout** | Timeout de 10s pour les requêtes HTTP |

---

## Dépendances CMake

```cmake
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)
target_link_libraries(ApplicationVide PRIVATE Qt6::Widgets Qt6::Network)
```

Qt6 est le seul framework externe. Pas de Boost, pas de bibliothèque tierce.
