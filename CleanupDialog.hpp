#ifndef CLEANUPDIALOG_HPP
#define CLEANUPDIALOG_HPP

#include <QDialog>
#include <QStringList>
#include "FolderCleaner.hpp"

class QListWidget;
class QCheckBox;
class QPushButton;
class QTextEdit;
class QLabel;

class CleanupDialog : public QDialog {
    Q_OBJECT
public:
    explicit CleanupDialog(const QStringList& initialPaths = {},
                           QWidget* parent = nullptr);

private slots:
    void addPaths();
    void removeSelected();
    void runAnalyze();
    void runClean();
    void onAnalysisProgress(const QString& message);
    void onAnalysisFinished(int emptyFiles, int thumbsDb, int emptyDirs);
    void onFileDeleted(const QString& path, const QString& operation);
    void onCleaningFinished(int emptyFiles, int thumbsDb, int emptyDirs);

private:
    void setButtonsEnabled(bool enabled);

    QListWidget* pathList_;
    QCheckBox* chkEmptyFiles_;
    QCheckBox* chkThumbsDb_;
    QCheckBox* chkEmptyDirs_;
    QPushButton* btnAnalyze_;
    QPushButton* btnClean_;
    QPushButton* btnClose_;
    QTextEdit* logEdit_;
    QLabel* summaryLabel_;

    FolderCleaner* cleaner_;
    bool running_ = false;
    bool analysisDone_ = false;

    int lastEmpty_ = 0;
    int lastThumbs_ = 0;
    int lastDirs_ = 0;
};

#endif
