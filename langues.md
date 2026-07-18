# Ajouter une langue — guide complet

## Principe

L'application utilise des fichiers texte simples pour les traductions.
**Aucune recompilation n'est nécessaire** pour ajouter une nouvelle langue.

```
ApplicationVide/
└── lang/                    # Dossier des langues (à côté de l'exécutable)
    ├── francais.txt         # Français
    ├── anglais.txt          # English
    └── espagnol.txt         # ← Nouvelle langue à créer
```

---

## 1. Créer le fichier de traduction

### 1.1. Nommer le fichier

Le nom du fichier (sans extension) est le **code langue** utilisé en interne.

```
espagnol.txt    → code "espagnol"
deutsch.txt     → code "deutsch"
italiano.txt    → code "italiano"
```

Placer le fichier dans le dossier `lang/` à côté de l'exécutable.

### 1.2. Format du fichier

```ini
# Commentaire (commence par #)
cle=valeur
```

Règles :
- Encodage **UTF-8** (obligatoire pour les accents et émojis)
- Une entrée par ligne
- `cle` = identifiant unique (ex: `menu.file.load`)
- `valeur` = texte traduit (peut contenir des émojis)
- Les lignes vides et les commentaires (`#`) sont ignorés
- Les espaces autour du `=` sont ignorés

### 1.3. Exemple complet

```ini
# Español — ApplicationVide
app.name=ApplicationVide

menu.file=&Archivo
menu.file.load=📂 Cargar proyecto
menu.file.save=💾 Guardar proyecto
menu.file.quit=❌ Salir

menu.tools=&Herramientas
menu.tools.toolbox=🧰 Caja de herramientas

menu.help=&?
menu.help.check_update=🔄 Buscar actualizaciones
menu.help.about=ℹ️ Acerca de

menu.language=&Idioma

toolbar.load=📂 Cargar
toolbar.save=💾 Guardar

status.ready=Listo
status.loaded=Proyecto cargado
status.saved=Proyecto guardado
status.up_to_date=Aplicación actualizada (v%s)
status.update_check=Buscando actualizaciones...
status.update_checking=Comprobando actualizaciones...
status.update_failed=Error al buscar actualizaciones
status.update_available=Actualización %s disponible
status.component_selected=Componente seleccionado

central.placeholder=Abra o cree un proyecto para empezar
dock.toolbox=Caja de herramientas
toolbox.title=Caja de herramientas

dialog.about.title=Acerca de
dialog.about.text=<h3>%s</h3><p>Versión %s</p><p>Aplicación Qt6 multiplataforma.</p>

dialog.update.title=Actualización disponible
dialog.update.message=Hay una nueva versión disponible: %s\n\nVersión actual: %s\n\n%s\n\n¿Quieres descargar la actualización?
dialog.update.yes=Descargar
dialog.update.no=Más tarde

dialog.error.load=No se puede cargar el archivo de proyecto.
dialog.error.save=No se puede guardar el archivo de proyecto.
```

### 1.4. Liste complète des clés

| Clé | Usage | Exemple français |
|---|---|---|
| `app.name` | Titre de la fenêtre | `ApplicationVide` |
| `menu.file` | Menu Fichier | `&Fichier` |
| `menu.file.load` | Charger un projet | `📂 Charger un projet` |
| `menu.file.save` | Sauvegarder le projet | `💾 Sauvegarder le projet` |
| `menu.file.quit` | Quitter | `❌ Quitter` |
| `menu.tools` | Menu Outils | `&Outils` |
| `menu.tools.toolbox` | Boîte à outils | `🧰 Boîte à outils` |
| `menu.help` | Menu ? | `&?` |
| `menu.help.check_update` | Vérifier mise à jour | `🔄 Vérifier les mises à jour` |
| `menu.help.about` | À propos | `ℹ️ À propos` |
| `menu.language` | Menu Langue | `&Langue` |
| `toolbar.load` | Barre d'outils charger | `📂 Charger` |
| `toolbar.save` | Barre d'outils sauvegarder | `💾 Sauvegarder` |
| `status.ready` | Barre d'état : prêt | `Prêt` |
| `status.loaded` | Projet chargé | `Projet chargé` |
| `status.saved` | Projet sauvegardé | `Projet sauvegardé` |
| `status.up_to_date` | Application à jour | `Application à jour (v%s)` |
| `status.update_check` | Vérification en cours | `Vérification des mises à jour...` |
| `status.update_checking` | Vérification déjà en cours | `Vérification des mises à jour en cours...` |
| `status.update_failed` | Échec vérification | `Vérification échouée` |
| `status.update_available` | Mise à jour dispo | `Mise à jour %s disponible` |
| `status.component_selected` | Composant sélectionné | `Composant sélectionné` |
| `central.placeholder` | Texte central | `Ouvrez ou créez un projet pour commencer` |
| `dock.toolbox` | Titre du dock | `Boîte à outils` |
| `toolbox.title` | Titre de la toolbox | `Boîte à outils` |
| `dialog.about.title` | Titre dialogue À propos | `À propos` |
| `dialog.about.text` | Texte À propos (HTML) | `<h3>%s</h3><p>Version %s</p>...` |
| `dialog.update.title` | Titre dialogue mise à jour | `Mise à jour disponible` |
| `dialog.update.message` | Message mise à jour | `Une nouvelle version est disponible...` |
| `dialog.update.yes` | Bouton oui | `Télécharger` |
| `dialog.update.no` | Bouton non | `Plus tard` |
| `dialog.error.load` | Erreur chargement | `Impossible de charger le fichier projet.` |
| `dialog.error.save` | Erreur sauvegarde | `Impossible de sauvegarder le fichier projet.` |

**Astuce :** copier `lang/anglais.txt` et renommer — toutes les clés sont déjà présentes,
il suffit de traduire les valeurs.

---

## 2. Ajouter le nom d'affichage (optionnel mais recommandé)

Le nom affiché dans le menu Langue est géré dans `LangueManager.cpp` :

```cpp
// Fichier : LangueManager.cpp
const QHash<QString, QString> LangueManager::displayNames_ = {
    {"francais", "Français"},
    {"anglais",  "English"},
    {"espagnol", "Español"},       // ← Ajouter ici
    {"deutsch",  "Deutsch"},        // ← Ajouter ici
    {"italiano", "Italiano"},       // ← Ajouter ici
};
```

Si le code langue n'est pas dans cette table, le nom du fichier (sans `.txt`)
est utilisé directement.

Tant que cette modification n'est pas faite, la langue apparaîtra avec son **nom de fichier**
(ex: `espagnol` au lieu de `Español`). Cela fonctionne mais c'est moins joli.

---

## 3. Ajouter la détection automatique (optionnel)

La détection automatique de la langue du système se fait dans `LangueManager.cpp` :

```cpp
QString LangueManager::detectSystemLanguage() {
    QString lang = QLocale::system().name().toLower();

    if (lang.startsWith("fr"))
        return "francais";
    if (lang.startsWith("en"))
        return "anglais";
    if (lang.startsWith("es"))
        return "espagnol";       // ← Ajouter ici
    if (lang.startsWith("de"))
        return "deutsch";        // ← Ajouter ici
    if (lang.startsWith("it"))
        return "italiano";       // ← Ajouter ici

    return "anglais";  // fallback
}
```

`QLocale::system().name()` retourne des valeurs comme :
- `"fr_FR"`, `"fr_BE"`, `"fr_CA"` → français
- `"en_US"`, `"en_GB"`, `"en_AU"` → anglais
- `"es_ES"`, `"es_MX"` → espagnol
- `"de_DE"`, `"de_AT"`, `"de_CH"` → allemand
- `"it_IT"`, `"it_CH"` → italien

---

## 4. Exemple complet : ajouter l'espagnol

### Étape 1 : Créer `lang/espagnol.txt`

Copier `lang/anglais.txt` → `lang/espagnol.txt`, traduire toutes les valeurs.

### Étape 2 : Ajouter le nom d'affichage (recompilation nécessaire)

Dans `LangueManager.cpp` :
```cpp
{"espagnol", "Español"},
```

### Étape 3 : Ajouter la détection automatique (recompilation nécessaire)

Dans `LangueManager.cpp` :
```cpp
if (lang.startsWith("es"))
    return "espagnol";
```

### Étape 4 : Tester

Lancer l'application → Menu Langue → `Español`

### Sans recompilation

Si vous sautez les étapes 2 et 3 :
- La langue apparaît dans le menu avec son nom de fichier (`espagnol`)
- L'utilisateur peut la sélectionner manuellement
- La détection automatique ne fonctionnera pas pour les systèmes en espagnol

---

## 5. Structure du dossier `lang/`

```
lang/
├── francais.txt        # Français (complet)
├── anglais.txt         # English (complet)
├── espagnol.txt        # Español (à créer)
├── deutsch.txt         # Deutsch (à créer)
├── italiano.txt        # Italiano (à créer)
└── ...                 # N'importe quel .txt = une nouvelle langue
```

---

## 6. Résumé des modifications par étape

| Étape | Fichier à modifier | Recompilation ? |
|---|---|---|
| Créer le fichier `.txt` | `lang/ma_langue.txt` (nouveau) | **Non** |
| Ajouter le nom d'affichage | `LangueManager.cpp` | Oui |
| Ajouter la détection auto | `LangueManager.cpp` | Oui |
| Ajouter `LANG_BASE_URL` (optionnel) | `AppConfig.hpp` | Oui |
| Ajouter les sources (`lang/`) | `CMakeLists.txt` (déjà fait pour `lang/*`) | Non |
| Copier dans le build | `file(COPY lang/ ...)` dans CMake | Reconfigure |

---

## 7. Auto-téléchargement des fichiers manquants

L'application peut **télécharger automatiquement** un fichier de langue s'il est
manquant dans le dossier `lang/`.

### Fonctionnement

1. Au lancement, l'application détecte la langue système (ou lit `application.ini`)
2. Elle cherche le fichier `lang/<code>.txt`
3. Si le fichier **n'existe pas**, elle le télécharge depuis :
   ```
   https://raw.githubusercontent.com/Fo170/ApplicationVide/main/lang/<code>.txt
   ```
4. Pendant le téléchargement, `anglais.txt` est utilisé comme fallback
5. Une fois téléchargé, la langue est chargée et les menus sont mis à jour
6. `application.ini` est mis à jour immédiatement

### Quand le téléchargement échoue (pas de réseau)

- L'application reste en anglais (fallback intégré)
- L'utilisateur peut changer de langue manuellement via le menu **Langue**
- Les fichiers déjà présents dans `lang/` fonctionnent normalement

### URL de base

L'URL de téléchargement est configurée dans `AppConfig.hpp` :

```cpp
#define LANG_BASE_URL "https://raw.githubusercontent.com/Fo170/ApplicationVide/main/lang/"
```

Pour pointer vers un dépôt différent ou un serveur privé, modifier cette seule ligne.

### Ajouter une langue directement sur GitHub (sans recompilation)

1. Créer le fichier `<code>.txt` dans le dossier `lang/` du dépôt GitHub
2. Pousser sur `main`
3. L'application existante pourra le télécharger automatiquement si :
   - `LANG_BASE_URL` dans `AppConfig.hpp` pointe vers ce dépôt
   - Un utilisateur dont la langue système correspond lance l'application

---

## 8. Régénération de `application.ini`

Le fichier `application.ini` (à côté de l'exécutable) stocke :
- `langue` : le code de la langue sélectionnée
- `geometry` : la taille et position de la fenêtre

### Comportement

| Situation | Action |
|---|---|
| Fichier supprimé | Recréé **immédiatement** au prochain lancement avec la langue détectée |
| Changement de langue | Écrit dans le fichier **immédiatement** |
| Fermeture de l'app | Écrit (sauvegarde de la géométrie) |
| Fichier inexistant + pas de réseau | Détection → anglais → fichier créé avec ces valeurs |

### Exemple de fichier généré

```ini
[General]
langue=francais
geometry=@ByteArray(\x00\x00\x00\xff...)
```

L'utilisateur peut éditer manuellement ce fichier pour forcer une langue :
```ini
langue=anglais
```

---

## 9. Pour les LLM : checklist

### Nouvelle langue statique (fichier livré avec l'app)

1. [ ] Copier un fichier existant (`anglais.txt`) en `lang/ma_langue.txt`
2. [ ] Traduire toutes les valeurs (pas les clés)
3. [ ] Ajouter `{"code", "Nom"}`, dans `displayNames_` dans `LangueManager.cpp`
4. [ ] Ajouter `if (lang.startsWith("xx")) return "code";` dans `detectSystemLanguage()`
5. [ ] Recompiler
6. [ ] Tester : lancer → menu Langue → nouvelle langue

### Nouvelle langue via téléchargement (sans recompilation)

1. [ ] Créer `lang/<code>.txt` sur le dépôt GitHub (branche `main`)
2. [ ] Pousser sur GitHub
3. [ ] Vérifier que `LANG_BASE_URL` dans `AppConfig.hpp` pointe vers `main`
4. [ ] Un utilisateur avec la langue système adéquate déclenchera le téléchargement auto
5. [ ] Alternative : menu Langue → sélectionner manuellement la langue
