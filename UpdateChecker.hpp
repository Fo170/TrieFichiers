#ifndef UPDATECHECKER_HPP
#define UPDATECHECKER_HPP

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

#endif
