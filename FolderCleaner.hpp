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

    bool isRunning() const;

signals:
    void analysisProgress(const QString& message);
    void analysisFinished(int emptyFiles, int thumbsDb, int emptyDirs);
    void fileDeleted(const QString& path, const QString& operation);
    void finished(int emptyFiles, int thumbsDb, int emptyDirs);

private:
    bool running_ = false;

    void walkForAnalysis(const QString& root,
                         bool checkEmptyFiles,
                         bool checkThumbsDb,
                         bool checkEmptyDirs,
                         int& totalEmpty,
                         int& totalThumbs,
                         int& totalDirs);

    int deleteEmptyFiles(const QString& path);
    int deleteThumbsDb(const QString& path);
    int deleteEmptyDirs(const QString& path);
};

#endif
