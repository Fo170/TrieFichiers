#ifndef STRIPDIALOG_HPP
#define STRIPDIALOG_HPP

#include <QDialog>
#include <QStringList>
#include "FolderCleaner.hpp"

class QListWidget;
class QLineEdit;
class QPushButton;
class QTextEdit;
class QLabel;

class StripDialog : public QDialog {
    Q_OBJECT
public:
    explicit StripDialog(const QStringList& initialPaths = {},
                         QWidget* parent = nullptr);

private slots:
    void addPaths();
    void removeSelected();
    void runAnalyze();
    void runApply();
    void onStripProgress(const QString& message);
    void onStripAnalyzed(int count);
    void onStripFinished(int renamed, int errors);

private:
    void setButtonsEnabled(bool enabled);

    QListWidget* pathList_;
    QLineEdit* extInput_;
    QPushButton* btnAnalyze_;
    QPushButton* btnApply_;
    QPushButton* btnClose_;
    QTextEdit* logEdit_;
    QLabel* summaryLabel_;

    FolderCleaner* cleaner_;
    bool running_ = false;
    bool analysisDone_ = false;
    int lastCount_ = 0;
};

#endif
