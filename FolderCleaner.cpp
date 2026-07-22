#include "FolderCleaner.hpp"
#include <QDirIterator>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QCoreApplication>

FolderCleaner::FolderCleaner(QObject* parent)
    : QObject(parent) {}

bool FolderCleaner::isRunning() const {
    return running_;
}

void FolderCleaner::analyze(const QStringList& paths,
                             bool checkEmptyFiles,
                             bool checkThumbsDb,
                             bool checkEmptyDirs) {
    running_ = true;
    int totalEmpty = 0, totalThumbs = 0, totalDirs = 0;

    for (const QString& path : paths) {
        QDir dir(path);
        if (!dir.exists()) {
            emit analysisProgress(QString("Ignore (introuvable) : %1").arg(path));
            continue;
        }
        walkForAnalysis(path, checkEmptyFiles, checkThumbsDb,
                        checkEmptyDirs, totalEmpty, totalThumbs, totalDirs);

        emit analysisProgress(QString("  => Fichiers vides : %1 | Thumbs.db : %2 | Dossiers vides : %3")
            .arg(totalEmpty).arg(totalThumbs).arg(totalDirs));
    }

    running_ = false;
    emit analysisFinished(totalEmpty, totalThumbs, totalDirs);
}

void FolderCleaner::walkForAnalysis(const QString& root,
                                     bool checkEmptyFiles,
                                     bool checkThumbsDb,
                                     bool checkEmptyDirs,
                                     int& totalEmpty,
                                     int& totalThumbs,
                                     int& totalDirs) {
    emit analysisProgress(root);

    QDir dir(root);
    QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);

    for (const QFileInfo& info : entries) {
        QCoreApplication::processEvents();

        if (info.isDir()) {
            walkForAnalysis(info.absoluteFilePath(),
                            checkEmptyFiles, checkThumbsDb, checkEmptyDirs,
                            totalEmpty, totalThumbs, totalDirs);
            if (checkEmptyDirs) {
                QDir sub(info.absoluteFilePath());
                if (sub.entryList(QDir::AllEntries | QDir::NoDotAndDotDot).isEmpty()) {
                    emit analysisProgress(QString("  [dossier vide] %1").arg(info.absoluteFilePath()));
                    ++totalDirs;
                }
            }
        } else if (info.isFile()) {
            if (checkEmptyFiles && info.size() == 0) {
                emit analysisProgress(QString("  [vide] %1").arg(info.absoluteFilePath()));
                ++totalEmpty;
            }
            if (checkThumbsDb && info.fileName() == "Thumbs.db") {
                emit analysisProgress(QString("  [Thumbs.db] %1").arg(info.absoluteFilePath()));
                ++totalThumbs;
            }
        }
    }
}

void FolderCleaner::clean(const QStringList& paths,
                           bool doEmptyFiles,
                           bool doThumbsDb,
                           bool doEmptyDirs) {
    running_ = true;
    int totalEmpty = 0, totalThumbs = 0, totalDirs = 0;

    for (const QString& path : paths) {
        QDir dir(path);
        if (!dir.exists()) continue;

        if (doEmptyFiles)
            totalEmpty += deleteEmptyFiles(path);
        if (doThumbsDb)
            totalThumbs += deleteThumbsDb(path);
        if (doEmptyDirs)
            totalDirs += deleteEmptyDirs(path);
    }

    running_ = false;
    emit finished(totalEmpty, totalThumbs, totalDirs);
}

void FolderCleaner::analyzeStrip(const QStringList& paths,
                                  const QString& extension) {
    running_ = true;
    int total = 0;

    for (const QString& path : paths) {
        emit stripProgress(QString("Analyse : %1").arg(path));
        walkForStrip(path, extension, total, true);
    }

    running_ = false;
    emit stripAnalyzed(total);
}

void FolderCleaner::applyStrip(const QStringList& paths,
                                const QString& extension) {
    running_ = true;
    int total = 0, errors = 0;

    for (const QString& path : paths) {
        QDir dir(path);
        if (!dir.exists()) continue;
        int before = total;
        walkForStrip(path, extension, total, false);
        int done = total - before;
        if (done > 0 || errors > 0)
            emit stripProgress(QString("  %1 renommes, %2 erreurs").arg(done).arg(errors));
    }

    running_ = false;
    emit stripFinished(total, errors);
}

void FolderCleaner::walkForStrip(const QString& root, const QString& ext,
                                  int& count, bool preview) {
    emit stripProgress(root);

    QDir dir(root);
    QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);

    for (const QFileInfo& info : entries) {
        QCoreApplication::processEvents();

        if (info.isDir()) {
            walkForStrip(info.absoluteFilePath(), ext, count, preview);
        } else if (info.isFile()) {
            QString name = info.fileName();
            if (name.endsWith(ext, Qt::CaseInsensitive)) {
                QString newName = name.left(name.length() - ext.length());
                if (preview) {
                    emit stripProgress(QString("  %1 -> %2").arg(name, newName));
                    ++count;
                } else {
                    QDir parent = info.absoluteDir();
                    if (parent.rename(info.absoluteFilePath(),
                                      parent.absolutePath() + "/" + newName)) {
                        emit stripProgress(QString("  %1 -> %2").arg(name, newName));
                        ++count;
                    } else {
                        emit stripProgress(QString("  ERREUR: %1").arg(name));
                    }
                }
            }
        }
    }
}

int FolderCleaner::deleteEmptyFiles(const QString& path) {
    int count = 0;
    QDirIterator it(path, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        QFileInfo info = it.fileInfo();
        if (info.size() == 0) {
            if (QFile::remove(info.absoluteFilePath())) {
                emit fileDeleted(info.absoluteFilePath(), "empty_file");
                ++count;
            }
        }
        QCoreApplication::processEvents();
    }
    return count;
}

int FolderCleaner::deleteThumbsDb(const QString& path) {
    int count = 0;
    QDirIterator it(path, QStringList{"Thumbs.db"}, QDir::Files,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        if (QFile::remove(it.fileInfo().absoluteFilePath())) {
            emit fileDeleted(it.fileInfo().absoluteFilePath(), "thumbs_db");
            ++count;
        }
        QCoreApplication::processEvents();
    }
    return count;
}

int FolderCleaner::deleteEmptyDirs(const QString& path) {
    int count = 0;
    QDirIterator it(path, QDir::Dirs | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);

    QStringList dirs;
    while (it.hasNext()) {
        it.next();
        dirs.prepend(it.fileInfo().absoluteFilePath());
    }

    for (const QString& dirPath : dirs) {
        QDir dir(dirPath);
        if (!dir.exists()) continue;
        if (dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot).isEmpty()) {
            if (dir.rmdir(dirPath)) {
                emit fileDeleted(dirPath, "empty_dir");
                ++count;
            }
        }
        QCoreApplication::processEvents();
    }
    return count;
}
