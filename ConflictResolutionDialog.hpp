#ifndef CONFLICTRESOLUTIONDIALOG_HPP
#define CONFLICTRESOLUTIONDIALOG_HPP

#include <QDialog>
#include <QString>
#include <QDateTime>

class QLabel;
class QPushButton;

class ConflictResolutionDialog : public QDialog {
    Q_OBJECT
public:
    enum Result {
        Overwrite,
        Skip,
        OverwriteAll,
        SkipAll,
        Cancel
    };

    explicit ConflictResolutionDialog(
        const QString& sourcePath,
        const QString& destPath,
        QWidget* parent = nullptr);

    Result chosenAction() const { return result_; }

private:
    void setupUi(const QString& sourcePath, const QString& destPath);
    QString formatSize(qint64 bytes) const;

    Result result_ = Skip;
    QPushButton* btnOverwrite_;
    QPushButton* btnSkip_;
    QPushButton* btnOverwriteAll_;
    QPushButton* btnSkipAll_;
    QPushButton* btnCancel_;
};

#endif
