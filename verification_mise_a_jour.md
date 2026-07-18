# Vérification de mises à jour — mode d'emploi

## Architecture

Le système de mise à jour repose sur ces fichiers :

```
ApplicationVide/
├── AppConfig.hpp             # Version courante + URLs (centralisé)
├── CMakeLists.txt            # Version CMake (PROJECT_VERSION, pour packaging)
├── UpdateChecker.hpp/.cpp    # Classe qui vérifie la version en ligne
└── version.json              # Fichier distant hébergé sur GitHub
```

### Flux

```
[Application]                    [GitHub]
     |                               |
     |-- GET version.json ---------->|
     |<-- {"version":"1.1.0",...} ---|
     |                               |
     |-- compare versions            |
     |   current < latest ?          |
     |   → notify user               |
```

---

## 1. Définir la version courante

La version est définie dans `AppConfig.hpp` (fichier centralisé) :

```cpp
// AppConfig.hpp
#define APP_VERSION "1.0.0"
```

C'est le seul endroit à modifier pour changer la version de l'application.
Le format attendu est `X.Y.Z` (sémantique).

Utilisation dans le code :

```cpp
#include "AppConfig.hpp"
QString versionCourante = QStringLiteral(APP_VERSION);
```

`CMakeLists.txt` conserve `PROJECT_VERSION` pour le packaging CMake, mais ce n'est pas utilisé par l'application :

```cmake
project(ApplicationVide VERSION 1.0.0 LANGUAGES CXX)
```

---

## 2. Créer le fichier `version.json`

Ce fichier doit être hébergé en ligne (GitHub, votre serveur, etc.).

### Structure

```json
{
    "version": "1.1.0",
    "url": "https://github.com/Fo170/ApplicationVide/releases/tag/v1.1.0",
    "notes": "Correction de bugs et améliorations"
}
```

| Champ | Obligatoire | Description |
|---|---|---|
| `version` | Oui | Version au format sémantique `X.Y.Z` |
| `url` | Non | Lien de téléchargement (ouvre le navigateur) |
| `notes` | Non | Description des changements |

### Hébergement sur GitHub

**Option A — dans le dépôt lui-même (recommandé) :**

Le fichier est commité à la racine du dépôt. URL d'accès brute :

```
https://raw.githubusercontent.com/UTILISATEUR/NOM_DU_REPO/BRANCHE/version.json
```

Exemple utilisé dans le projet :

```
https://raw.githubusercontent.com/Fo170/ApplicationVide/main/version.json
```

**Option B — Dans une GitHub Release :**

Lors de la création d'une release, le fichier `version.json` peut être téléchargé depuis les assets.

**Option C — Sur un serveur dédié :**

Hébergez le fichier sur n'importe quel serveur HTTP.

### Important

Le fichier `version.json` doit toujours contenir la **dernière version disponible**.
Pensez à le mettre à jour **après chaque release**.

---

## 3. Utilisation de `UpdateChecker`

L'URL de vérification et la version sont définies dans `AppConfig.hpp` :
```cpp
#define APP_VERSION "1.0.0"
#define UPDATE_CHECK_URL "https://raw.githubusercontent.com/UTILISATEUR/REPO/main/version.json"
```

### Constructeur

```cpp
UpdateChecker(
    const QString& currentVersion,  // Version locale (ex: APP_VERSION)
    const QString& checkUrl,        // URL du version.json distant (ex: UPDATE_CHECK_URL)
    QObject* parent = nullptr
);
```

### Méthodes

```cpp
void checkForUpdates();   // Déclenche la vérification
bool isChecking() const;  // Vérification en cours ?
```

### Signaux

```cpp
// Une nouvelle version est disponible
void updateAvailable(
    const QString& latestVersion,   // "1.1.0"
    const QString& downloadUrl,     // lien vers la release
    const QString& releaseNotes     // notes de version
);

// L'application est à jour
void upToDate();

// Erreur de vérification (pas de réseau, fichier invalide...)
void checkError(const QString& errorMessage);
```

### Exemple d'intégration

```cpp
#include "UpdateChecker.hpp"

// Créer le checker
checker_ = new UpdateChecker(
    QStringLiteral(APP_VERSION),
    QStringLiteral("https://raw.githubusercontent.com/UTILISATEUR/REPO/main/version.json"),
    this
);

// Connecter les signaux
connect(checker_, &UpdateChecker::updateAvailable, this, [this](
    const QString& version, const QString& url, const QString& notes) {

    QString msg = QString(
        "Nouvelle version : %1\n"
        "Actuelle : %2\n\n%3")
        .arg(version, APP_VERSION, notes);

    if (QMessageBox::question(this, "Mise à jour", msg) == QMessageBox::Yes)
        QDesktopServices::openUrl(QUrl(url));
});

connect(checker_, &UpdateChecker::upToDate, this, []() {
    statusBar()->showMessage("Application à jour");
});

connect(checker_, &UpdateChecker::checkError, this, [](const QString& err) {
    statusBar()->showMessage("Erreur de mise à jour : " + err);
});

// Lancer la vérification
checker_->checkForUpdates();
```

---

## 4. Quand vérifier ?

| Moment | Action |
|---|---|
| Au démarrage de l'application | Vérification silencieuse en arrière-plan |
| Action manuelle (menu `?`) | Vérification avec indication visuelle |

Ne pas bloquer l'interface pendant la vérification
→ `QNetworkAccessManager` est **asynchrone**.

---

## 5. Workflow complet de release

### Créer une nouvelle version

1. Mettre à jour la version dans `AppConfig.hpp` (centralisé) :
   ```cpp
   #define APP_VERSION "1.1.0"
   ```

2. (Optionnel) Mettre à jour `CMakeLists.txt` pour le packaging :
   ```cmake
   project(ApplicationVide VERSION 1.1.0 LANGUAGES CXX)
   ```

3. Mettre à jour `version.json` :
   ```json
   {
       "version": "1.1.0",
       "url": "https://github.com/Fo170/ApplicationVide/releases/tag/v1.1.0",
       "notes": "Nouvelle fonctionnalité X, correction de Y"
   }
   ```

4. Compiler et tester
5. Commiter et pusher sur `main` :
   ```bash
   git add AppConfig.hpp CMakeLists.txt version.json
   git commit -m "Release v1.1.0"
   git tag v1.1.0
   git push && git push --tags
   ```

6. Créer une **GitHub Release** avec le binaire compilé
7. **Vérifier** que `version.json` sur `main` a bien été mis à jour

### Que se passe-t-il côté utilisateur ?

- Si version locale = `1.0.0` et `version.json` dit `"1.1.0"` → notification mise à jour
- Si version locale = `1.1.0` et `version.json` dit `"1.1.0"` → "Application à jour"
- Si version locale = `1.2.0` et `version.json` dit `"1.1.0"` → considéré à jour
  (version locale supérieure à la version distante)
- Pas de réseau → signal `checkError` → message en barre d'état

---

## 6. Comparaison des versions

`QVersionNumber` compare correctement les versions sémantiques :

```
1.0.0  < 1.0.1
1.0.9  < 1.1.0
1.1.0  < 2.0.0
2.0    < 2.0.0  (non, 2.0 == 2.0.0)
```

**Attention :** le versionning doit être cohérent.
Préférer le format `X.Y.Z` (majeur.mineur.patch).

---

## 7. Gestion des erreurs

| Erreur | Cause probable | Action |
|---|---|---|
| `HostNotFound` | Pas de connexion internet | Ignorer silencieusement |
| `Timeout` | Serveur lent ou bloqué | Réessayer plus tard |
| `ContentNotFound` | Fichier version.json supprimé/déplacé | Vérifier l'URL |
| `Format invalide` | JSON mal formé | Valider sur jsonlint.com |
| `Champ version manquant` | JSON incomplet | Vérifier la structure |
| `Version non parseable` | Mauvais format ("v1.0" au lieu de "1.0") | Utiliser X.Y.Z |

---

## 8. Personnalisation

### Ajouter une vérification automatique périodique

```cpp
#include <QTimer>

// Dans le constructeur
QTimer* timer = new QTimer(this);
connect(timer, &QTimer::timeout, checker_, &UpdateChecker::checkForUpdates);
timer->start(86400000); // 24h
```

### Afficher la version dans la barre de titre

```cpp
setWindowTitle(QString("ApplicationVide v%1").arg(APP_VERSION));
```

### Forcer la mise à jour silencieuse

```cpp
// Lancer sans notification sonore
connect(checker_, &UpdateChecker::upToDate, this, []() {
    // Ne rien faire, silencieux
});
```

---

## 9. Contenu du `CMakeLists.txt` (extrait)

```cmake
project(ApplicationVide VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

add_executable(ApplicationVide
    main.cpp
    MainWindow.cpp
    UpdateChecker.cpp
    resources.qrc
)

target_link_libraries(ApplicationVide PRIVATE Qt6::Widgets Qt6::Network)
```

---

## 10. Résumé

| Élément | Fichier | Rôle |
|---|---|---|---|
| Version courante + URLs | `AppConfig.hpp` | `APP_VERSION`, `UPDATE_CHECK_URL` (centralisé) |
| Vérification | `UpdateChecker.hpp/.cpp` | Appel HTTP, parsing JSON, comparaison |
| Intégration | `MainWindow.cpp` | Menu "?", signal au démarrage |
| Version distante | `version.json` sur GitHub | Fichier JSON hébergé en ligne |
| Téléchargement | Navigateur via `QDesktopServices` | Lien vers GitHub Releases |

---

## 11. Checklist pour un LLM

Quand vous devez implémenter ou configurer la vérification de mises à jour :

1. [ ] Définir `APP_VERSION` dans `AppConfig.hpp`
2. [ ] Définir `UPDATE_CHECK_URL` dans `AppConfig.hpp` (URL du `version.json`)
3. [ ] Ajouter `Qt6::Network` dans `find_package` et `target_link_libraries`
4. [ ] Créer `UpdateChecker.hpp` avec la classe, les signaux, les slots
5. [ ] Créer `UpdateChecker.cpp` avec `QNetworkAccessManager`, parsing JSON, comparaison avec `QVersionNumber`
6. [ ] Ajouter `UpdateChecker.cpp` aux sources de `add_executable`
7. [ ] Créer l'instance dans le constructeur avec `APP_VERSION` et `UPDATE_CHECK_URL`
8. [ ] Connecter les 3 signaux (`updateAvailable`, `upToDate`, `checkError`)
9. [ ] Créer `version.json` à la racine et le pousser sur GitHub
10. [ ] Ajouter un menu "Vérifier les mises à jour" dans l'UI
11. [ ] Lancer `checkForUpdates()` au démarrage
12. [ ] Mettre à jour `version.json` à chaque release
