# Guide d'implantation — 6 fonctionnalités Qt

Ce guide explique comment implémenter dans une application Qt6 les six fonctionnalités présentes dans ce projet. Chaque section est autonome : vous pouvez piocher celle qui vous intéresse.

---

## 1. Icône d'application (Windows + Linux)

### Principe

Deux mécanismes complémentaires :
- **Qt** : des PNG multi-tailles embarqués via le système de ressources → visibles dans la barre de titre, la barre des tâches, Alt+Tab
- **Windows PE** : un fichier `.ico` lié via `.rc` → visible dans l'Explorateur de fichiers

### Ce qu'il faut créer

```
ico/
├── app-32.png        # Icône Qt 32px
├── app-64.png        # Icône Qt 64px
├── app-128.png       # Icône Qt 128px
├── app-256.png       # Icône Qt 256px
├── app.ico           # Icône Windows (32+64px)
└── app.svg           # Icône vectorielle (optionnel, Linux)
```

### 1.1. Ressources Qt (`.qrc`)

Fichier `resources.qrc` :

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

Les fichiers sont accessibles via le préfixe `:/ico/`.

### 1.2. Chargement dans `main.cpp`

```cpp
#include <QApplication>
#include <QIcon>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QIcon icone;
    icone.addFile(":/ico/app-256.png", QSize(256, 256));
    icone.addFile(":/ico/app-128.png", QSize(128, 128));
    icone.addFile(":/ico/app-64.png", QSize(64, 64));
    icone.addFile(":/ico/app-32.png", QSize(32, 32));
    app.setWindowIcon(icone);

    MainWindow fenetre;
    fenetre.setWindowIcon(icone);  // nécessaire aussi sur la fenêtre
    fenetre.show();

    return app.exec();
}
```

### 1.3. Icône Windows PE (Explorateur)

Fichier `app.rc` :

```
IDI_ICON1 ICON "ico/app.ico"
```

Dans `CMakeLists.txt`, pré-compiler la ressource avec `windres` (MinGW) :

```cmake
set(WINDRES "C:/Qt/Tools/mingw1310_64/bin/windres.exe")
set(RC_OUT "${CMAKE_CURRENT_BINARY_DIR}/app_icon.o")
if(NOT EXISTS "${RC_OUT}")
    execute_process(
        COMMAND ${WINDRES} -i "${CMAKE_CURRENT_SOURCE_DIR}/app.rc" -o "${RC_OUT}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif()

# Puis inclure ${RC_OUT} dans add_executable()
```

> **Note** : ce `windres` est une astuce propre à MinGW pour contourner un problème de chemin. Sous MSVC, on utiliserait directement le compilateur de ressources (`rc.exe`). Sous Linux, l'icône `.ico` est ignorée, c'est le SVG qu'on utilise dans le fichier `.desktop`.

### 1.4. CMake : copier les fichiers dans le build

```cmake
file(COPY ico/app.ico DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ico)
```

---

## 2. Boîte de dialogue « À propos »

### Principe

Une simple `QMessageBox::about()` avec du texte HTML, appelée depuis une action du menu `?`.

### Code

```cpp
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include "AppConfig.hpp"   // pour APP_NAME, APP_VERSION, APP_HOMEPAGE_URL

void MainWindow::show_about() {
    QString title = "À propos";
    QString text = QString("<h3>%1</h3><p>Version %2</p>"
                           "<p>Mon application Qt6.</p>"
                           "<p><a href='%3'>%3</a></p>")
        .arg(APP_NAME)
        .arg(APP_VERSION)
        .arg(APP_HOMEPAGE_URL);
    QMessageBox::about(this, title, text);
}
```

### Intégration dans le menu

```cpp
// Dans create_menus()
menu_aide_ = menuBar()->addMenu("?");
a_about_ = menu_aide_->addAction("À propos");
connect(a_about_, &QAction::triggered, this, &MainWindow::show_about);
```

### Dépendances

- `Qt6::Widgets` (pour `QMessageBox`)
- `Qt6::Network` (uniquement si vous utilisez `QDesktopServices::openUrl`)

---

## 3. Détection de mise à jour

### Principe

1. Un fichier `version.json` hébergé en ligne contient `{ "version": "X.Y.Z", "url": "...", "notes": "..." }`
2. L'application télécharge ce JSON via HTTP
3. Elle compare avec sa version locale via `QVersionNumber`
4. Trois signaux possibles : `updateAvailable`, `upToDate`, `checkError`

### Classe `UpdateChecker`

**UpdateChecker.hpp** :

```cpp
#include <QObject>
#include <QString>
#include <QVersionNumber>

class QNetworkAccessManager;
class QNetworkReply;

class UpdateChecker : public QObject {
    Q_OBJECT
public:
    explicit UpdateChecker(const QString& currentVersion,
                           const QString& checkUrl,
                           QObject* parent = nullptr);
    void checkForUpdates();
    bool isChecking() const;

signals:
    void updateAvailable(const QString& latestVersion,
                         const QString& downloadUrl,
                         const QString& releaseNotes);
    void upToDate();
    void checkError(const QString& errorMessage);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager* manager_;
    QString currentVersion_;
    QString checkUrl_;
    bool checking_ = false;
};
```

**UpdateChecker.cpp** (extrait) :

```cpp
void UpdateChecker::checkForUpdates() {
    if (checking_) return;
    checking_ = true;

    QNetworkRequest request{QUrl(checkUrl_)};
    request.setHeader(QNetworkRequest::UserAgentHeader, "MonApp/1.0");
    request.setTransferTimeout(10000);  // 10s timeout
    manager_->get(request);
}

void UpdateChecker::onReplyFinished(QNetworkReply* reply) {
    checking_ = false;

    if (reply->error() != QNetworkReply::NoError) {
        emit checkError(reply->errorString());
        reply->deleteLater();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();

    QJsonObject obj = doc.object();
    QString latestVer = obj["version"].toString();
    QString downloadUrl = obj["url"].toString();

    QVersionNumber current = QVersionNumber::fromString(currentVersion_);
    QVersionNumber latest = QVersionNumber::fromString(latestVer);

    if (latest > current)
        emit updateAvailable(latestVer, downloadUrl, obj["notes"].toString());
    else
        emit upToDate();
}
```

### Utilisation dans `MainWindow`

```cpp
update_checker_ = new UpdateChecker(APP_VERSION, UPDATE_CHECK_URL, this);
connect(update_checker_, &UpdateChecker::updateAvailable, this, [&](...) {
    // Afficher QMessageBox avec bouton "Télécharger"
});
connect(update_checker_, &UpdateChecker::upToDate, this, [&]() {
    statusBar()->showMessage("Application à jour");
});
connect(update_checker_, &UpdateChecker::checkError, this, [&](const QString& err) {
    statusBar()->showMessage("Échec : " + err);
});
```

### Fichier `version.json` distant

```json
{
    "version": "1.0.0",
    "url": "https://github.com/user/repo/releases",
    "notes": "Description de la version"
}
```

Héberger sur GitHub raw (ou tout serveur HTTPS).

---

## 4. Sauvegarde des paramètres (`.ini`)

### Principe

`QSettings` avec le format INI permet de lire/écrire un fichier texte à côté de l'exécutable. Idéal pour : langue, géométrie de fenêtre, préférences.

### Sauvegarde

```cpp
#include <QSettings>
#include <QCoreApplication>

void MainWindow::save_settings() {
    QSettings settings(
        QCoreApplication::applicationDirPath() + "/application.ini",
        QSettings::IniFormat);
    settings.setValue("langue", langue_);
    settings.setValue("geometry", saveGeometry());
}
```

### Chargement

```cpp
void MainWindow::load_settings() {
    QSettings settings(
        QCoreApplication::applicationDirPath() + "/application.ini",
        QSettings::IniFormat);

    QString lang = settings.value("langue", "").toString();
    if (lang.isEmpty())
        lang = detectSystemLanguage();

    QByteArray geo = settings.value("geometry").toByteArray();
    if (!geo.isEmpty())
        restoreGeometry(geo);
}
```

### Sauvegarde automatique à la fermeture

```cpp
void MainWindow::closeEvent(QCloseEvent* event) {
    save_settings();
    QMainWindow::closeEvent(event);
}
```

### Comportement

| Événement | Action |
|---|---|
| Fichier manquant au lancement | Créé automatiquement après `load_settings()` |
| Changement de préférence | `save_settings()` appelé immédiatement |
| Fermeture de l'application | `save_settings()` appelé dans `closeEvent()` |

Le fichier généré ressemble à :

```ini
[General]
langue=francais
geometry=@ByteArray(\x00\x00\x00\xff...)
```

---

## 5. Gestion multi-langue

### Principe

Une classe `LangueManager` qui :
- Charge un fichier texte `lang/<code>.txt` (format `clé=valeur`, UTF-8)
- Stocke les traductions dans un `QHash<QString, QString>`
- Émet `languageChanged()` pour que l'UI se retraduise
- Détecte la langue système via `QLocale`
- Télécharge les fichiers manquants depuis une URL distante

### Fichier de langue (ex: `lang/francais.txt`)

```ini
# Français — MonApp
app.name=MonApp
menu.file=&Fichier
menu.file.load=📂 Charger
status.ready=Prêt
dialog.about.title=À propos
```

### Classe `LangueManager`

```cpp
class LangueManager : public QObject {
    Q_OBJECT
public:
    explicit LangueManager(const QString& langDir, QObject* parent = nullptr);

    bool load(const QString& languageCode);
    QString get(const QString& key) const;
    QString currentLanguage() const;
    QStringList availableLanguages() const;
    QStringList languageDisplayNames() const;
    static QString detectSystemLanguage();
    void downloadLanguage(const QString& code, const QString& remoteUrl);

signals:
    void languageChanged();
    void languageDownloaded(const QString& code, bool success);

private:
    QHash<QString, QString> translations_;
    QString langDir_;
    QString currentLang_;
    QNetworkAccessManager* network_;
    static const QHash<QString, QString> displayNames_;
};
```

### Chargement d'un fichier

```cpp
bool LangueManager::load(const QString& languageCode) {
    QFile f(langDir_ + "/" + languageCode + ".txt");
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QHash<QString, QString> newTranslations;
    QTextStream in(&f);
    in.setEncoding(QStringConverter::Utf8);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#'))
            continue;
        int sep = line.indexOf('=');
        if (sep <= 0) continue;
        newTranslations[line.left(sep).trimmed()] = line.mid(sep + 1).trimmed();
    }
    f.close();

    translations_ = newTranslations;
    currentLang_ = languageCode;
    emit languageChanged();
    return true;
}
```

### Détection de la langue système

```cpp
static QString LangueManager::detectSystemLanguage() {
    QString lang = QLocale::system().name().toLower();
    if (lang.startsWith("fr")) return "francais";
    if (lang.startsWith("en")) return "anglais";
    return "anglais";  // fallback
}
```

### Utilisation dans `MainWindow`

```cpp
// Construction
langue_ = new LangueManager(
    QCoreApplication::applicationDirPath() + "/lang", this);

// Connexion pour retraduction automatique
connect(langue_, &LangueManager::languageChanged,
        this, &MainWindow::retranslateUi);

// Retraduction
void MainWindow::retranslateUi() {
    menu_fichier_->setTitle(langue_->get("menu.file"));
    a_charger_->setText(langue_->get("menu.file.load"));
    // ... tous les textes de l'UI
}
```

### Menu de sélection de langue (dynamique)

```cpp
void MainWindow::retranslateUi() {
    // ...
    menu_langue_->clear();
    auto* group = new QActionGroup(this);
    QStringList codes = langue_->availableLanguages();
    QStringList names = langue_->languageDisplayNames();
    QString current = langue_->currentLanguage();

    for (int i = 0; i < codes.size(); ++i) {
        auto* a = menu_langue_->addAction(names.at(i));
        a->setCheckable(true);
        a->setChecked(codes.at(i) == current);
        a->setData(codes.at(i));
        group->addAction(a);
        connect(a, &QAction::triggered, this, [this, code = codes.at(i)]() {
            changer_langue(code);
        });
    }
}
```

### Auto-téléchargement

```cpp
void LangueManager::downloadLanguage(const QString& code,
                                     const QString& remoteUrl) {
    QNetworkReply* reply = network_->get(QNetworkRequest(QUrl(remoteUrl)));
    connect(reply, &QNetworkReply::finished, this, [this, reply, code]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit languageDownloaded(code, false);
            return;
        }
        QDir().mkpath(langDir_);
        QFile f(langDir_ + "/" + code + ".txt");
        if (!f.open(QIODevice::WriteOnly)) {
            emit languageDownloaded(code, false);
            return;
        }
        f.write(reply->readAll());
        f.close();
        emit languageDownloaded(code, true);
    });
}
```

### Ajouter une nouvelle langue (sans recompilation)

1. Créer `lang/espagnol.txt` avec toutes les clés traduites
2. Optionnel (recompilation) : ajouter dans `displayNames_` et `detectSystemLanguage()`

---

## 6. Charger et sauvegarder un projet JSON

### Principe

Une classe `Project` qui encapsule un `QJsonObject` avec :
- `version` : format du fichier
- `name` : nom du projet
- `components[]` : tableau de composants

### Classe `Project`

**Project.hpp** :

```cpp
class Project {
public:
    Project();

    bool load(const QString& path);
    bool save(const QString& path) const;

    void set_name(const QString& name);
    QString name() const;
    void add_component(const QJsonObject& component);
    QJsonArray components() const;
    void set_data(const QJsonObject& data);
    QJsonObject data() const;
    bool is_modified() const;
    void set_modified(bool modified);
    QString file_path() const;

private:
    QJsonObject data_;
    QString file_path_;
    bool modified_ = false;
};
```

### Implémentation

```cpp
Project::Project() {
    data_["version"] = "1.0";
    data_["name"] = "Nouveau projet";
    data_["components"] = QJsonArray();
}

bool Project::load(const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();

    if (!doc.isObject()) return false;

    data_ = doc.object();
    file_path_ = path;
    modified_ = false;
    return true;
}

bool Project::save(const QString& path) const {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly))
        return false;

    QJsonDocument doc(data_);
    f.write(doc.toJson(QJsonDocument::Indented));
    f.close();

    const_cast<Project*>(this)->file_path_ = path;
    const_cast<Project*>(this)->modified_ = false;
    return true;
}
```

### Intégration dans `MainWindow`

```cpp
void MainWindow::charger_projet() {
    QString path = QFileDialog::getOpenFileName(
        this, "Charger un projet", QString(),
        "Projet JSON (*.json);;Tous (*)");
    if (path.isEmpty()) return;

    if (!project_.load(path)) {
        QMessageBox::warning(this, "Erreur",
            "Impossible de charger le fichier.");
        return;
    }
    update_title();
}

void MainWindow::sauvegarder_projet() {
    QString path = QFileDialog::getSaveFileName(
        this, "Sauvegarder le projet", QString(),
        "Projet JSON (*.json);;Tous (*)");
    if (path.isEmpty()) return;

    if (!project_.save(path)) {
        QMessageBox::warning(this, "Erreur",
            "Impossible de sauvegarder le fichier.");
        return;
    }
    update_title();
}
```

### Exemple de fichier produit

```json
{
    "version": "1.0",
    "name": "Mon Projet",
    "components": [
        {
            "type": "capteur",
            "nom": "Température"
        }
    ]
}
```

---

## Résumé des dépendances Qt

| Fonctionnalité | Classes Qt | Module CMake |
|---|---|---|
| Icônes | `QIcon`, `QSize`, `QApplication` | `Qt6::Widgets` |
| À propos | `QMessageBox`, `QDesktopServices` | `Qt6::Widgets` |
| Mise à jour | `QNetworkAccessManager`, `QNetworkReply`, `QNetworkRequest`, `QJsonDocument`, `QJsonObject`, `QVersionNumber` | `Qt6::Network` |
| Fichier `.ini` | `QSettings` | `Qt6::Widgets` (transitif) |
| Multi-langue | `QHash`, `QTextStream`, `QFile`, `QLocale`, `QNetworkAccessManager` | `Qt6::Widgets` + `Qt6::Network` |
| JSON | `QJsonDocument`, `QJsonObject`, `QJsonArray`, `QFile` | `Qt6::Widgets` (transitif) |

---

# Pièges et bonnes pratiques — Qt GUI

Cette section détaille les erreurs fréquentes et les solutions pour un fonctionnement fiable.

---

## 7. Affichage correct des émojis

### 7.1. Encodage des fichiers source

Les émojis dans le code C++ ne posent pas de problème **si** le compilateur interprète le fichier source en UTF-8.

| Compilateur | Comportement par défaut | Solution |
|---|---|---|
| **GCC / MinGW** | UTF-8 par défaut | Aucune action nécessaire |
| **MSVC** | ANSI (locale système) | Ajouter dans `CMakeLists.txt` : `target_compile_options(ApplicationVide PRIVATE /utf-8)` |
| **Clang** | UTF-8 par défaut | Aucune action nécessaire |

**Piège MSVC** : sans le flag `/utf-8`, un émoji comme 📂 écrit dans un `"text"` littéral devient un charabia. Symptôme : des caractères corrompus apparaissent au lieu des émojis.

### 7.2. Encodage des fichiers de langue (`.txt`)

```cpp
// OBLIGATOIRE pour lire les émojis depuis un fichier
QTextStream in(&f);
in.setEncoding(QStringConverter::Utf8);
```

**Sans cette ligne**, `QTextStream` utilise l'encodage système (Latin1 sur Windows français), et les émojis sont détruits.

### 7.3. Pas de BOM dans les fichiers `.txt`

Un BOM UTF-8 (3 octets `EF BB BF` en début de fichier) peut perturber la première clé. `QTextStream` le détecte et l'ignore, mais certains éditeurs l'ajoutent. Préférer **UTF-8 sans BOM** (option standard dans VS Code, Notepad++, Qt Creator).

### 7.4. Police de caractères

Qt gère automatiquement le **fallback** vers une police émoji :
- **Windows** → `Segoe UI Emoji`
- **Linux** → `Noto Color Emoji` (doit être installé)
- **macOS** → `Apple Color Emoji`

Aucune configuration manuelle nécessaire. Si les émojis n'apparaissent pas sous Linux :
```bash
sudo apt install fonts-noto-color-emoji    # Debian/Ubuntu
sudo dnf install google-noto-color-emoji-fonts  # Fedora
```

### 7.5. Émojis dans les menus et toolbar

Exemple fonctionnel :
```cpp
auto* action = menu->addAction(QStringLiteral("📂 Charger"));
// ou via fichier de langue :
// menu.file.load=📂 Charger un projet
```

`QStringLiteral` garantit la compilation en UTF-8. Ne pas utiliser `QString::fromLatin1()` ou `QString::fromLocal8Bit()` avec des émojis.

### 7.6. Émojis dans `QMessageBox`

`QMessageBox::about()` et `QMessageBox::information()` acceptent nativement les émojis dans leurs chaînes. Aucun traitement spécial.

---

## 8. Pièges CMake et compilation

### 8.1. `CMAKE_AUTOMOC` et `CMAKE_AUTORCC` obligatoires

```cmake
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
```

Sans `AUTOMOC` : les classes avec `Q_OBJECT` ne compileront pas (signaux/slots non générés).
Sans `AUTORCC` : le fichier `.qrc` ne sera pas compilé → ressources introuvables.

### 8.2. `Q_OBJECT` doit être dans le `.hpp`, pas dans le `.cpp`

```cpp
// Correct : dans le header
class MainWindow : public QMainWindow {
    Q_OBJECT
    // ...
};
```

Si `Q_OBJECT` est dans le `.cpp`, `AUTOMOC` ne le détecte pas toujours. Mettez-le toujours dans le header.

### 8.3. `windres` et l'icône Windows PE

Le chemin vers `windres` est **hardcodé** dans `CMakeLists.txt` :
```cmake
set(WINDRES "C:/Qt/Tools/mingw1310_64/bin/windres.exe")
```

**Ce chemin change à chaque version de Qt/MinGW.** Si vous mettez à jour Qt, l'icône PE disparaît de l'Explorateur. Solution : cibler le bon dossier ou rendre le chemin configurable.

L'icône `.o` n'est générée qu'**à la configuration** (`cmake -S . -B windows`), pas à la compilation. Si vous supprimez le dossier `windows/`, relancez `cmake` avant `cmake --build`.

### 8.4. Ordre dans `target_link_libraries`

```cmake
target_link_libraries(ApplicationVide PRIVATE Qt6::Widgets Qt6::Network)
```

L'ordre n'a pas d'importance avec `PRIVATE` et les cibles IMPORTED de Qt, mais évitez d'oublier `Qt6::Network` si vous utilisez `QNetworkAccessManager`.

### 8.5. Copie des fichiers dans le dossier de build

```cmake
file(COPY lang/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/lang)
file(COPY ico/app.ico DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ico)
```

Ces fichiers sont copiés **à la configuration**, pas à chaque compilation. Si vous les modifiez, relancez `cmake`. Pour une synchro à chaque build, remplacer par `configure_file()` ou un script `add_custom_command()`.

### 8.6. Chemin des ressources dans `.qrc`

Les chemins dans `resources.qrc` sont **relatifs au fichier `.qrc` lui-même**, pas à la racine du projet :

```
ApplicationVide/
├── resources.qrc          # ← point de référence
├── ico/
│   └── app-256.png        # → s'écrit "ico/app-256.png" dans le .qrc
```

Préfixe d'accès après compilation : `:/ico/app-256.png`.

---

## 9. Pièges Qt runtime

### 9.1. `QStringLiteral` vs `QString()`

| Expression | Action | À utiliser |
|---|---|---|
| `QStringLiteral("texte")` | Compile le littéral en UTF-16 à la compilation | **Toujours** pour les chaînes fixes |
| `QString("texte")` | Conversion runtime via `QString::fromUtf8()` | Éviter (sauf si paramètre variable) |
| `QLatin1String("texte")` | Latin1, pas d'UTF-8 | Jamais avec des émojis ou accents |

```cpp
// Bon
setWindowTitle(QStringLiteral("Mon App"));

// Mauvais (conversion inutile à chaque exécution)
setWindowTitle(QString("Mon App"));

// Très mauvais (perd les émojis)
setWindowTitle(QLatin1String("📂 Charger"));
```

### 9.2. `deleteLater()` sur les réponses réseau

```cpp
void onReplyFinished(QNetworkReply* reply) {
    // Traiter reply...
    reply->deleteLater();  // ← OBLIGATOIRE, même en cas d'erreur
}
```

Sans `deleteLater()`, le `QNetworkReply` fuit mémoire. Le `deleteLater()` permet à Qt de terminer proprement le traitement de l'événement avant libération.

### 9.3. Timeout réseau

```cpp
request.setTransferTimeout(10000);  // 10 secondes, en millisecondes
```

**Sans timeout**, une requête HTTP vers un serveur injoignable peut attendre indéfiniment (120s par défaut sous Windows). Ajoutez toujours un timeout.

Disponible depuis Qt 5.15. Sous Qt 5.12, utiliser un `QTimer` séparé.

### 9.4. Parent Qt et gestion mémoire

```cpp
// La fenêtre est parent du dock → suppression automatique
dock_toolbox_ = new QDockWidget(this);

// Idem pour les layouts : pas besoin de delete manuel
auto* lay = new QVBoxLayout(this);
```

Règle : si vous passez un `parent` à un QObject, **vous ne devez jamais appeler `delete` dessus**. À la destruction du parent, tous les enfants sont détruits. Ne pas passer de parent = gestion manuelle.

### 9.5. Chemin du fichier `.ini`

```cpp
QSettings settings(
    QCoreApplication::applicationDirPath() + "/application.ini",
    QSettings::IniFormat);
```

`applicationDirPath()` retourne le dossier de l'exécutable, pas le dossier de travail courant. C'est le bon emplacement pour un fichier de configuration.
Attention : ce dossier peut ne pas être accessible en écriture (ex : installé dans `Program Files`). Sur Windows moderne, préférer `QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)` pour un déploiement sérieux.

### 9.6. `closeEvent` et sauvegarde

```cpp
void MainWindow::closeEvent(QCloseEvent* event) {
    save_settings();                           // 1. Sauver les réglages
    QMainWindow::closeEvent(event);            // 2. Propager l'événement (OBLIGATOIRE)
}
```

Sans `QMainWindow::closeEvent(event)`, la fenêtre ne se ferme pas.

### 9.7. `QVariantMap` pour les données utilisateur

```cpp
// Transmettre des données structurées avec un signal
QVariantMap data;
data["nom"] = "Capteur";
data["type"] = "capteur";
emit component_selected(data);

// Les récupérer dans le slot
void onComponentSelected(const QVariantMap& data) {
    QString nom = data.value("nom").toString();
    QString type = data.value("type").toString();
}
```

Alternative : créer une classe dédiée. `QVariantMap` est pratique pour des prototypes ou des données simples.

### 9.8. `QVersionNumber` et versioning sémantique

```cpp
QVersionNumber current = QVersionNumber::fromString("1.0.0");   // OK
QVersionNumber latest  = QVersionNumber::fromString("1.0.0");   // OK
// "1.0" fonctionne aussi (interprété comme 1.0.0)

QVersionNumber a = QVersionNumber::fromString("v1.0.0");  // ← NULL (échec)
QVersionNumber b = QVersionNumber::fromString("1.0");      // OK
```

Le format attendu est `X.Y.Z` sans préfixe. Un `v` en tête (ex: `v1.0.0`) retourne une version nulle et `isNull()` est vrai.

### 9.9. `QTextStream::setEncoding` obligatoire pour l'UTF-8

```cpp
// Dans Qt6 :
in.setEncoding(QStringConverter::Utf8);

// NE PAS utiliser (Qt5 déprécié) :
// in.setCodec("UTF-8");  // ← Qt5 only, ne compile plus sous Qt6
```

Sous Qt6, `QTextCodec` a été supprimé. Utilisez `QStringConverter::Utf8`.

### 9.10. JSON indenté

```cpp
f.write(doc.toJson(QJsonDocument::Indented));
```

Sans `Indented`, le JSON est produit sur une seule ligne → illisible pour l'utilisateur.

---

## 10. Pièges spécifiques aux plateformes

### 10.1. Windows : segments de chemin (`\` vs `/`)

Qt normalise automatiquement les séparateurs, mais dans les `#include`, utilisez `/` :
```cpp
#include "lang/LangueManager.hpp"  // OK partout
// #include "lang\LangueManager.hpp"  // KO sous Linux
```

### 10.2. Windows : dépendances DLL (`windeployqt`)

```bash
windeployqt windows/ApplicationVide.exe
```

Cette commande copie toutes les DLL Qt nécessaires dans le dossier de l'exécutable. Si vous lancez l'exe sans ça, il ne démarrera pas (erreur "DLL manquante").
À exécuter après chaque `cmake --build`.

### 10.3. Windows : Qt Platform Theme

Sans les plugins de plateforme, l'application refuse de démarrer :
```
This application failed to start because no Qt platform plugin could be initialized.
```

`windeployqt` copie automatiquement le dossier `platforms/` (avec `qwindows.dll` dedans).

### 10.4. Linux : fichiers `.desktop` et icône

Sous Linux, l'icône visible dans le lanceur d'applications nécessite un fichier `.desktop` :

```ini
[Desktop Entry]
Name=ApplicationVide
Exec=/usr/local/bin/ApplicationVide
Icon=/usr/local/share/icons/ApplicationVide/app.svg
Type=Application
Categories=Development;
```

L'icône `.svg` du dossier `ico/` peut être utilisée. L'icône `.ico` est ignorée sous Linux.

### 10.5. Linux : polices émoji

```bash
# Vérifier si une police émoji est installée
fc-list | grep -i emoji

# Installer
sudo apt install fonts-noto-color-emoji
```

Sans police émoji, les émojis s'affichent comme des carrés blancs (☐).

---

## 11. Checklist finale : « Est-ce que mon Qt GUI va marcher ? »

Avant de compiler votre application Qt6, vérifiez :

### CMakeLists.txt
- [ ] `CMAKE_AUTOMOC ON` et `CMAKE_AUTORCC ON`
- [ ] `find_package(Qt6 REQUIRED COMPONENTS Widgets Network)` (ajoutez Network si besoin)
- [ ] Tous les fichiers `.cpp` et `.qrc` dans `add_executable()`
- [ ] `target_link_libraries(... PRIVATE Qt6::Widgets Qt6::Network)`
- [ ] `/utf-8` sous MSVC : `target_compile_options(... PRIVATE /utf-8)`
- [ ] `file(COPY lang/ DESTINATION ...)` pour les ressources externes

### Sources
- [ ] `Q_OBJECT` dans tous les headers de classes avec signaux/slots
- [ ] `QStringLiteral` pour les chaînes fixes, pas `QString("...")`
- [ ] `reply->deleteLater()` dans tous les handlers réseau
- [ ] `setTransferTimeout()` sur chaque requête réseau
- [ ] `setEncoding(QStringConverter::Utf8)` sur les `QTextStream`
- [ ] `QMainWindow::closeEvent(event)` dans `closeEvent()`
- [ ] `Indented` dans `toJson()` pour fichiers utilisateur
- [ ] Pas de `delete` manuel sur les QObject avec parent

### Ressources
- [ ] `.qrc` contient tous les fichiers PNG
- [ ] Chemins dans `.qrc` relatifs au fichier `.qrc`
- [ ] `app.rc` existe et pointe vers `ico/app.ico`
- [ ] `windres` chemin à jour (vérifier après mise à jour Qt)
- [ ] Fichiers `.txt` en UTF-8 **sans BOM**

### Runtime
- [ ] Lancer `windeployqt` avant distribution Windows
- [ ] Police Noto Color Emoji installée sous Linux
- [ ] `application.ini` accessible en écriture
- [ ] `version.json` accessible en ligne
- [ ] DLL OpenSSL présentes si utilisation d'HTTPS

---

## 12. HTTPS / SSL — OpenSSL sur Windows

### Le problème

Qt utilise OpenSSL pour les requêtes HTTPS. Sous Linux, OpenSSL est installé nativement. **Sous Windows, vous devez fournir les DLL.**

Symptôme si les DLL manquent :
```
qt.network.ssl: QSslSocket::connectToHostEncrypted: TLS initialization failed
```
ou bien le `QNetworkReply` retourne une erreur SSL sans message clair.

### DLL nécessaires

À copier dans le même dossier que l'exécutable (via `windeployqt` ou manuellement) :

```
openssl-3.dll
libcrypto-3-x64.dll
libssl-3-x64.dll
```

Sous Qt 6, le nom exact des DLL varie selon la version d'OpenSSL liée. `windeployqt` les copie automatiquement si présentes dans le répertoire Qt/bin.

### Vérification

```cpp
#include <QSslSocket>
qDebug() << "SSL support:" << QSslSocket::supportsSsl();
qDebug() << "SSL library:" << QSslSocket::sslLibraryBuildVersionString();
```

Si `supportsSsl()` retourne `false` → DLL manquantes ou incompatibles.

### Téléchargement officiel

```
https://slproweb.com/products/Win32OpenSSL.html
```

Choisir la version correspondant à votre Qt (regarder `qDebug()` de `sslLibraryBuildVersionString()` pour connaître la version attendue).

---

## 13. Haute résolution (High DPI / écran 4K)

### Qt6 : comportement par défaut

Qt6 active **automatiquement** les écrans haute résolution. Plus besoin de flags :
```cpp
// Qt5 nécessitait :
// QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);  // Inutile sous Qt6
```

### Désactiver le scaling (si nécessaire)

```cpp
// Forcer le scaling à 1.0 (tout en pixels réels)
qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
// Ou via variable d'environnement :
// export QT_ENABLE_HIGHDPI_SCALING=0
```

### Tester

- Vérifier que les icônes 32px ne sont pas floues sur un écran 4K
- `QIcon::addFile(":/ico/app-256.png", QSize(256, 256));` choisira automatiquement la bonne taille
- Les polices sont mises à l'échelle automatiquement par Qt

### Piège : icônes de la toolbar

```cpp
tb->setIconSize(QSize(16, 16));  // pixels logiques, pas pixels physiques
```

Qt adapte automatiquement. Si l'icône est trop petite sur écran 4K, fournissez des tailles plus grandes (32, 48, 64px) et Qt choisira la meilleure.

### Variables d'environnement utiles

| Variable | Effet |
|---|---|
| `QT_SCALE_FACTOR=1.5` | Forcer un facteur d'échelle |
| `QT_AUTO_SCREEN_SCALE_FACTOR=0` | Désactiver le scaling automatique |
| `QT_ENABLE_HIGHDPI_SCALING=0` | Désactiver complètement le High DPI |
| `QT_SCREEN_SCALE_FACTORS=HDMI1=1.5;eDP1=1.25` | Scaling par écran |

---

## 14. Débogage Qt

### 14.1. `qDebug()` et flux de débogage

```cpp
#include <QDebug>

qDebug() << "Valeur:" << variable;
qWarning() << "Attention:" << raison;
qCritical() << "Erreur fatale:" << message;
// qFatal("Message");  // ← termine l'application
```

`qDebug()` écrit sur `stderr`. Sous Windows, la sortie est visible dans la console du débogueur (VS Code, Qt Creator) ou dans `DebugView` (Sysinternals).

### 14.2. Activer les logs Qt

Pour voir ce qui se passe dans Qt (plugins réseau, SSL, ressources, etc.) :

```bash
# Variables d'environnement
export QT_DEBUG_PLUGINS=1          # Voir le chargement des plugins (platforme, image, etc.)
export QT_LOGGING_RULES="qt.network.ssl=true"  # Logs SSL détaillés
export QT_LOGGING_RULES="qt.network*=true"     # Tous les logs réseau

# Sous Windows PowerShell :
$env:QT_DEBUG_PLUGINS=1
$env:QT_LOGGING_RULES="qt.network*=true"
```

### 14.3. Vérifier qu'une ressource QRC existe

```cpp
bool existe = QFile::exists(":/ico/app-256.png");
qDebug() << "Ressource trouvée:" << existe;

// Lister toutes les ressources disponibles (Qt6)
QDirIterator it(":/", QDir::Files | QDir::NoDotAndDotDot,
                QDirIterator::Subdirectories);
while (it.hasNext()) {
    qDebug() << "Ressource:" << it.next();
}
```

### 14.4. Erreur fréquente : plugin de plateforme manquant

```
qt.qpa.plugin: Could not load the Qt platform plugin "windows" in "" even though it was found.
```

Solutions :
1. Lancer `windeployqt` (copie `platforms/qwindows.dll`)
2. Variable : `export QT_QPA_PLATFORM_PLUGIN_PATH=/chemin/vers/plugins`
3. Forcer un platforme : `-platform offscreen` (sans GUI, pour tests)

### 14.5. Erreur réseau SSL

```
qt.network.ssl: QSslSocket::connectToHostEncrypted: TLS initialization failed
```

Voir section 12 (OpenSSL). Diagnostiquer avec :
```cpp
qDebug() << "SSL support:" << QSslSocket::supportsSsl();
qDebug() << "Build version:" << QSslSocket::sslLibraryBuildVersionString();
qDebug() << "Runtime version:" << QSslSocket::sslLibraryVersionString();
```

### 14.6. Vérifier la version Qt à l'exécution

```cpp
qDebug() << "Qt version:" << qVersion();          // "6.8.0"
qDebug() << "Build ABI:" << QSysInfo::buildAbi(); // "x86_64-little_endian-llp64"
qDebug() << "OS:" << QSysInfo::prettyProductName();
```

---

## 15. Signaux / slots avancés

### 15.1. Connexions directes vs files d'attente

```cpp
// Même thread (défaut) → DirectConnection
connect(obj, &Class::signal, this, &Class::slot);

// Thread différent → Qt::QueuedConnection (automatique si objets dans threads différents)
connect(obj, &Class::signal, this, &Class::slot, Qt::QueuedConnection);
```

**Règle** : ne jamais modifier l'UI depuis un thread non principal. Si votre `UpdateChecker` ou `LangueManager` émet un signal depuis un thread secondaire, Qt bascule automatiquement en `QueuedConnection` si le récepteur est dans le thread principal.

### 15.2. Piège : lambda et captures par référence

```cpp
// DANGEREUX : la référence peut être invalide quand la lambda s'exécute
connect(reply, &QNetworkReply::finished, this, [this, &code]() {
    // &code peut déjà être détruit si code est une variable locale
});

// SÛR : capture par copie
connect(reply, &QNetworkReply::finished, this, [this, code]() {
    // code est copié dans la lambda
});

// SÛR aussi : capture par déplacement (C++14)
connect(reply, &QNetworkReply::finished, this, [this, code = std::move(code)]() {});
```

### 15.3. Déconnexion automatique avec `Qt::SingleShotConnection`

Disponible depuis Qt 6.0 :

```cpp
connect(obj, &Class::signal, this, &Class::slot,
        Qt::SingleShotConnection);
// La connexion est automatiquement rompue après la première émission.
// Utile pour les téléchargements uniques de langue, les réponses attendues une fois.
```

### 15.4. Slot lambda et contexte (`context`)

```cpp
// La connexion est rompue si `this` est détruit (automatique avec parent Qt)
connect(obj, &Class::signal, this, [this]() {
    // this est valide tant que la connexion existe
});

// Si connexion à un objet qui n'est pas un parent :
connect(obj, &Class::signal, receiver.get(), [receiver = receiver.get()]() {
    // receiver est un QPointer, connexion rompue si détruit
});
```

### 15.5. Bloquer temporairement un signal

```cpp
obj->blockSignals(true);
// Modifier l'objet sans déclencher ses signaux
obj->blockSignals(false);
```

Utile par exemple pour mettre à jour un champ sans redéclencher la validation.

---

## 16. Boucle d'événements et threads

### 16.1. Ne jamais bloquer le thread principal

```cpp
// À ÉVITER :
QThread::sleep(3);                    // Bloque toute l'UI pendant 3s
while (condition) { /* attente active */ }  // 100% CPU, UI figée

// À LA PLACE :
// 1. QTimer pour une action retardée
QTimer::singleShot(3000, this, [this]() {
    // action après 3s
});

// 2. Traitement long dans un thread séparé
QThread* worker = new QThread;
QObject* task = new MyTask;
task->moveToThread(worker);
worker->start();
```

### 16.2. `QTimer` pour actions périodiques

```cpp
// Vérification de mise à jour toutes les heures
auto* timer = new QTimer(this);
timer->setInterval(3600000);  // 1 heure
connect(timer, &QTimer::timeout, this, &MainWindow::verifier_mise_a_jour);
timer->start();
```

### 16.3. `QCoreApplication::processEvents()` — à utiliser avec précaution

```cpp
// Forcer le traitement des événements en attente
QCoreApplication::processEvents();
// Utile dans une longue boucle pour ne pas figer l'UI
// MAIS : peut causer de la réentrance (le slot peut être appelé pendant l'exécution)
```

Préférer un `QThread` ou `QtConcurrent::run()` pour un traitement long.

---

## 17. Qt6 vs Qt5 — breaking changes

Si vous portez du code Qt5 vers Qt6, ces changements vous concernent :

| Qt5 | Qt6 | Impact |
|---|---|---|
| `QStringRef` | `QStringView` | `QStringRef` supprimé, remplacer par `QStringView` |
| `QTextCodec::setCodecForLocale()` | `QStringConverter` | `QTextCodec` supprimé, voir section 9.9 |
| `QRegExp` | `QRegularExpression` | `QRegExp` supprimé, utiliser `QRegularExpression` |
| `QString::SkipEmptyParts` | `Qt::SkipEmptyParts` | Déplacé dans l'espace de noms Qt |
| `QNetworkAccessManager::finished(QNetworkReply*)` | Fonctionnel mais déprécié | Préférer connexion par reply (voir section 3) |
| `QApplication::setAttribute(Qt::AA_EnableHighDpiScaling)` | Inutile | High DPI activé par défaut dans Qt6 |
| `QSysInfo::WindowsVersion` | Supprimé | Utiliser `QSysInfo::productVersion()` |
| `QString::trimmed()` | Fonctionnel inchangé | RAS |
| `QString::mid()` | Déprécié | Préférer `QStringView::sliced()` ou `first()`/`last()` |
| `QVariant::operator<<` / `operator>>` | Supprimé | Utiliser `QVariant::fromValue()` / `QVariant::value<T>()` |
| `QTimeZone::availableTimeZoneIds()` | Déplacé | Signature modifiée |
| `QLocale::countryToString()` | Déprécié | Utiliser `QLocale::territoryToString()` |

### Exemple de migration `QTextCodec` → `QStringConverter`

```cpp
// Qt5 (ne compile plus sous Qt6)
QTextStream in(&f);
in.setCodec("UTF-8");

// Qt6
QTextStream in(&f);
in.setEncoding(QStringConverter::Utf8);
```

### Exemple de migration `QRegExp` → `QRegularExpression`

```cpp
// Qt5
QRegExp re("^\\d+$");
re.exactMatch(str);

// Qt6
QRegularExpression re("^\\d+$");
re.match(str).hasMatch();
```

---

## 18. Feuilles de style Qt (QSS)

### Principe

Qt supporte le CSS pour styliser les widgets. La syntaxe est proche du CSS web, avec des sélecteurs basés sur les classes Qt.

### Exemples

```cpp
// Texte central grisé
central_label_->setStyleSheet("color: #888; font-size: 14px;");

// Bouton avec survol
button->setStyleSheet(
    "QPushButton { background-color: #0078d4; color: white; }"
    "QPushButton:hover { background-color: #106ebe; }"
);

// Style global de l'application
app.setStyleSheet(
    "QMainWindow { background-color: #f5f5f5; }"
    "QMenuBar { background-color: #ffffff; border-bottom: 1px solid #ddd; }"
    "QToolBar { background-color: #ffffff; border: none; spacing: 4px; }"
    "QStatusBar { background-color: #e8e8e8; }"
);
```

### Pièges QSS

- Les sélecteurs utilisent le nom de la **classe Qt** (`QPushButton`, `QLabel`), pas le nom de votre classe
- Pour cibler une instance spécifique : `QPushButton#monBouton { ... }` avec `button->setObjectName("monBouton")`
- QSS ne supporte pas toutes les propriétés CSS (ex: `flexbox`, `grid` inexistants)
- Pour des thèmes complets (dark mode), utiliser `app.setStyleSheet()` au niveau global

### Exemple : dark theme minimal

```cpp
app.setStyleSheet(
    "QMainWindow, QDialog, QWidget { background-color: #2b2b2b; color: #e0e0e0; }"
    "QMenuBar, QToolBar { background-color: #333; color: #e0e0e0; }"
    "QMenu { background-color: #333; color: #e0e0e0; }"
    "QMenu::selected { background-color: #0078d4; }"
    "QTreeWidget { background-color: #252525; color: #e0e0e0; }"
    "QStatusBar { background-color: #333; color: #aaa; }"
);
```

---

## 19. Gestion des erreurs réseau et SSL

### 19.1. Distinguer les types d'erreur

```cpp
void onReplyFinished(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        switch (reply->error()) {
        case QNetworkReply::ConnectionRefusedError:
            // Serveur refusé (pas de réseau local, service down)
            break;
        case QNetworkReply::HostNotFoundError:
            // DNS échoué (nom d'hôte inexistant)
            break;
        case QNetworkReply::TimeoutError:
            // Délai d'attente dépassé
            break;
        case QNetworkReply::SslHandshakeFailedError:
            // Problème de certificat SSL
            break;
        default:
            // Autre erreur
            break;
        }
    }
}
```

### 19.2. Ignorer les erreurs SSL (développement uniquement)

```cpp
// NE JAMAIS utiliser en production (interception possible)
connect(reply, &QNetworkReply::sslErrors, this,
        [](const QList<QSslError>& errors) {
    qWarning() << "SSL errors ignored (dev mode):" << errors;
    reply->ignoreSslErrors();  // ← passe outre
});
```

### 19.3. Vérifier le code HTTP

```cpp
int statusCode = reply->attribute(
    QNetworkRequest::HttpStatusCodeAttribute).toInt();
// 200 = OK, 404 = introuvable, 500 = erreur serveur
```

Utile pour distinguer un fichier distant manquant (404) d'une vraie erreur réseau.

---

## 20. Déploiement avancé

### 20.1. `windeployqt` — fichiers copiés

```bash
windeployqt windows/ApplicationVide.exe
```

Copie automatiquement :
- DLL Qt (`Qt6Core.dll`, `Qt6Widgets.dll`, `Qt6Network.dll`, etc.)
- Plugins : `platforms/qwindows.dll`, `styles/qwindowsvistastyle.dll`,
  `tls/qopensslbackend.dll` (si OpenSSL présent), `imageformats/*.dll`
- Fichiers de traduction Qt (`.qm`)
- OpenSSL DLL (si présentes dans Qt/bin)

### 20.2. Redistribuable MSVC

Si compilé avec MSVC, l'exécutable a besoin du **Microsoft Visual C++ Redistributable**.
- Inclure `vc_redist.x64.exe` dans l'installateur
- Ou lier statiquement avec `/MT` (déconseillé : plus volumineux, moins flexible)

### 20.3. Installateur minimal (NSIS)

```nsis
; Exemple NSIS
OutFile "Setup-ApplicationVide.exe"
InstallDir "$PROGRAMFILES64\ApplicationVide"

Section "Install"
    SetOutPath "$INSTDIR"
    File "windows\ApplicationVide.exe"
    File /r "windows\*.dll"
    File /r "windows\platforms"
    File /r "windows\styles"
    File /r "windows\tls"
    File /r "windows\imageformats"
    File /r "windows\lang"
    File "windows\ico\app.ico"
SectionEnd
```

### 20.4. Application portable (clé USB)

Même principe que l'installateur, mais sans installer. Copier tout le dossier de sortie de `windeployqt` sur une clé USB. L'application fonctionne sans installation.

### 20.5. Réduction de la taille

```bash
# Supprimer les DLL inutiles après windeployqt
rm Qt6Qml.dll            # Si pas de QML
rm Qt6Quick.dll          # Si pas de Qt Quick
rm Qt6Svg.dll            # Si pas de SVG
rm Qt6Test.dll           # Si pas de tests
rm Qt6WebEngine*         # Si pas de web engine
rm -rf qml/              # Si pas de QML
```

Sauvegarde typique : 50-80 Mo → 15-25 Mo.

---

## 21. Technologies Qt utilisées dans AppNeurone

### Modules CMake

```
Qt6::Widgets    — UI complète (fenêtres, widgets, graphics, événements)
Qt6::Network    — Mise à jour, téléchargement langues
```

### Arborescence des classes Qt

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

### Résumé de l'architecture Qt

- **Pas de Qt Quick/QML** — 100% widgets Qt classiques
- **Rendu 2D custom** via `QPainter` sur des `QGraphicsObject`
- **Pas de Qt3D/OpenGL** — tout en 2D logicielle
- **CMAKE_AUTOMOC** activé (signaux/slots)
- **CMAKE_AUTORCC** activé (ressources `.qrc`)
- **Ressources** embarquées (icônes PNG)
- **Polices** : configuration du fallback émoji dans `main.cpp`

---

## 22. Correction des problèmes d'émoji (noir et blanc → couleur)

### Problème

Les emoji (`🧠`, `📂`, `💾`, etc.) s'affichaient en noir et blanc dans toute l'interface Qt (menus, toolbars, labels, status bar) après une mise à jour de Qt.

### Cause

Sous Windows, `QFont::setFamilies()` permet de définir une liste de polices de secours. Le code original faisait :

```cpp
QFont f = app.font();
QStringList fam = f.families();       // → ["Segoe UI", ...]
fam << "Noto Color Emoji" << "Symbola" << "Segoe UI Emoji";
f.setFamilies(fam);
app.setFont(f);
```

Le problème : `Segoe UI` (première police de la liste) contient ses propres glyphes d'emoji, mais en **noir et blanc**. Qt utilise la première police qui supporte le caractère, donc il prend le glyphe B&W de `Segoe UI` et ne descend jamais jusqu'à `Segoe UI Emoji`.

Sous Qt 6.3+ (qui utilise DirectWrite au lieu de GDI), cette chaîne de fallback explicite **écrase** la chaîne native de DirectWrite qui, elle, sait associer `Segoe UI` pour le texte et `Segoe UI Emoji` pour les emoji colorés automatiquement.

### Solution

Mettre les polices emoji **avant** `Segoe UI` dans la chaîne, pour que les caractères emoji trouvent `Segoe UI Emoji` en premier :

```cpp
QFont f = app.font();
QStringList fam = f.families();
// Retirer les polices emoji si déjà présentes
fam.removeAll("Segoe UI Emoji");
fam.removeAll("Noto Color Emoji");
fam.removeAll("Apple Color Emoji");
fam.removeAll("Symbola");
// Les ajouter en tête de liste
fam.push_front("Segoe UI Emoji");
fam.push_front("Noto Color Emoji");
fam.push_front("Apple Color Emoji");
fam.push_front("Symbola");
f.setFamilies(fam);
app.setFont(f);
```

Ordre final de la chaîne de fallback :

1. `Segoe UI Emoji` → rend les emoji en couleur, lettres/chiffres en caractères texte standard
2. `Noto Color Emoji` → fallback Linux
3. `Apple Color Emoji` → fallback macOS
4. `Symbola` → fallback générique
5. Polices système d'origine (`Segoe UI`, etc.) → fallback ultime

**Note importante** : `Segoe UI Emoji` contient les lettres latines et chiffres comme des **glyphes texte standards**, pas comme des emoji. Les caractères ASCII (`a`, `1`, `#`) ne deviennent pas des pictogrammes colorés.

### Cas particulier : barres de titre Windows

Les titres de fenêtres sont dessinés par Windows (zone non-client du gestionnaire de fenêtres), pas par Qt. Windows utilise `Segoe UI` (B&W) pour la barre de titre, et `Segoe UI Emoji` n'est jamais consulté. Les emoji dans les titres restent donc **toujours en noir et blanc** — c'est une limitation du système, pas de Qt.

Solutions pour les titres :
- Accepter le B&W (simple, cohérent avec les autres applications Windows)
- Remplacer les emoji par du texte
- Utiliser une barre de titre personnalisée (`Qt::FramelessWindowHint` + widget titre dessiné par Qt) — beaucoup plus complexe

---

## 23. Erreurs DLL récurrentes — conflit de versions Qt

### Symptômes

```
Le point d'entrée de procédure _Z9qBadAllocv est introuvable
dans la bibliothèque de liens dynamiques.

Le point d'entrée de procédure _Z21qRegisterResourceDataiPKhS0_S0_
est introuvable dans la bibliothèque de liens dynamiques.
```

Ces symboles C++ manglés correspondent respectivement à `qBadAlloc()` et `qRegisterResourceData(int, unsigned char const*, unsigned char const*, unsigned char const*)` — deux fonctions exportées par `Qt6Core.dll`.

### Cause

L'exécutable a été compilé avec une version de Qt (ex: Qt 6.11.0) mais charge au runtime les DLL Qt d'une **version différente** (ex: Qt 6.2, Qt 6.5). Les symboles exportés changent entre versions mineures de Qt 6.

Scénarios typiques :

| Scénario | Cause |
|----------|-------|
| Build réutilisé avec DLL d'une ancienne installation | `windeployqt` non exécuté après rebuild |
| Projet partagé entre machines avec Qt versions différentes | DLL d'une version A, binaire compilé avec version B |
| Mise à jour de Qt sans rebuild complet | Cache CMake + DLL obsolètes |

### Solution

#### 1. Nettoyer et rebuild

```bash
# Supprimer le cache CMake et les DLL
rm -rf qtgui/build/CMakeCache.txt qtgui/build/CMakeFiles
rm -f qtgui/build/*.dll qtgui/build/*.exe

# Reconfigurer
cmake -S qtgui -B qtgui/build -G Ninja \
    -DCMAKE_PREFIX_PATH="C:/Qt/6.11.0/mingw_64" \
    -DCMAKE_CXX_COMPILER="C:/Qt/Tools/mingw1310_64/bin/g++.exe" \
    -DCMAKE_MAKE_PROGRAM="C:/Qt/Tools/Ninja/ninja.exe"

# Recompiler
cmake --build qtgui/build
```

#### 2. Déployer les DLL de la bonne version avec windeployqt

```bash
# Déployer les DLL Qt correspondant EXACTEMENT à la version de compilation
windeployqt qtgui/build/AppNeurone.exe
```

`windeployqt` copie automatiquement :
- `Qt6Core.dll`, `Qt6Gui.dll`, `Qt6Widgets.dll`, `Qt6Network.dll`
- Plugins : `platforms/qwindows.dll`, `styles/`, `tls/`, `imageformats/`, etc.
- Les DLL OpenSSL si présentes (optionnel)
- Fichiers de traduction Qt (`.qm`)

Les DLL copiées proviennent du même dossier Qt que celui utilisé pour la compilation (ex: `C:/Qt/6.11.0/mingw_64/bin/`).

#### 3. Vérification

```bash
# Lister les DLL du dossier build
ls qtgui/build/Qt*.dll

# Vérifier la version (sous Windows, clic droit → Propriétés → Détails)
# Ou avec un outil comme Dependency Walker
```

#### 4. Prévention

- Toujours exécuter `windeployqt` après un `cmake --build`
- Ne pas mélanger des DLL de versions Qt différentes dans le même dossier
- Après une mise à jour de Qt, toujours rebuild from scratch (supprimer `CMakeCache.txt`)
- Si le projet est partagé via git, ne pas versionner les DLL — chaque développeur exécute `windeployqt` localement
