#ifndef APPCONFIG_HPP
#define APPCONFIG_HPP

#include <QString>

// ── Nom de l'application ─────────────────────────────────────────────────
#define APP_NAME "ApplicationVide"

// ── Version de l'application ─────────────────────────────────────────────
// Modifier ici à chaque release (format X.Y.Z)
#ifndef APP_VERSION
#define APP_VERSION "1.0.0"
#endif

// ── URLs ─────────────────────────────────────────────────────────────────
// URL du fichier version.json pour la vérification de mise à jour
// Doit pointer vers un fichier JSON avec les champs : version, url, notes
#define UPDATE_CHECK_URL "https://raw.githubusercontent.com/Fo170/ApplicationVide/main/version.json"

// URL du dépôt GitHub (page d'accueil, releases, etc.)
#define APP_HOMEPAGE_URL "https://github.com/Fo170/ApplicationVide"

// ── Icônes ───────────────────────────────────────────────────────────────
// Toutes les icônes sont dans le dossier "ico/" à la racine du projet.
// Formats supportés :
//   - PNG (32, 64, 128, 256 px) → Qt resources (barre de titre, tâche, Alt+Tab)
//   - ICO (32, 64 px)           → Ressource Windows PE (Explorateur de fichiers)
//   - SVG (optionnel)           → Linux .desktop, thèmes d'icônes
//
// Le dossier "ico/" contient :
//   app-32.png     Icône Qt 32px
//   app-64.png     Icône Qt 64px
//   app-128.png    Icône Qt 128px
//   app-256.png    Icône Qt 256px
//   app.ico        Icône Windows (32+64px)
//   app.svg        Icône vectorielle (optionnel, Linux)
//
// Préfixe Qt resource : ":/ico/"

#endif
