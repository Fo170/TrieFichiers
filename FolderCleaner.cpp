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

    if (checkEmptyFiles) {
        QDirIterator it(root, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            if (it.fileInfo().size() == 0) {
                emit analysisProgress(QString("  [vide] %1").arg(it.fileInfo().absoluteFilePath()));
                ++totalEmpty;
            }
            QCoreApplication::processEvents();
        }
    }

    if (checkThumbsDb) {
        QDirIterator it(root, QStringList{"Thumbs.db"}, QDir::Files,
                        QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            emit analysisProgress(QString("  [Thumbs.db] %1").arg(it.fileInfo().absoluteFilePath()));
            ++totalThumbs;
            QCoreApplication::processEvents();
        }
    }

    if (checkEmptyDirs) {
        QDirIterator it(root, QDir::Dirs | QDir::NoDotAndDotDot,
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
                emit analysisProgress(QString("  [dossier vide] %1").arg(dirPath));
                ++totalDirs;
            }
            QCoreApplication::processEvents();
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
