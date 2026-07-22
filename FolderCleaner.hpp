#ifndef FOLDERCLEANER_HPP
#define FOLDERCLEANER_HPP

#include <QObject>
#include <QStringList>

class FolderCleaner : public QObject {
    Q_OBJECT
public:
    explicit FolderCleaner(QObject* parent = nullptr);

    void analyze(const QStringList& paths,
                 bool checkEmptyFiles,
                 bool checkThumbsDb,
                 bool checkEmptyDirs);

    void clean(const QStringList& paths,
               bool doEmptyFiles,
               bool doThumbsDb,
               bool doEmptyDirs);

    void analyzeStrip(const QStringList& paths, const QString& extension);
    void applyStrip(const QStringList& paths, const QString& extension);

    bool isRunning() const;

signals:
    void analysisProgress(const QString& message);
    void analysisFinished(int emptyFiles, int thumbsDb, int emptyDirs);
    void fileDeleted(const QString& path, const QString& operation);
    void finished(int emptyFiles, int thumbsDb, int emptyDirs);
    void stripProgress(const QString& message);
    void stripAnalyzed(int count);
    void stripFinished(int renamed, int errors);

private:
    bool running_ = false;

    void walkForAnalysis(const QString& root,
                         bool checkEmptyFiles,
                         bool checkThumbsDb,
                         bool checkEmptyDirs,
                         int& totalEmpty,
                         int& totalThumbs,
                         int& totalDirs);

    void walkForStrip(const QString& root, const QString& ext,
                      int& count, bool preview);

    int deleteEmptyFiles(const QString& path);
    int deleteThumbsDb(const QString& path);
    int deleteEmptyDirs(const QString& path);
};

#endif
