#include "UpdateChecker.hpp"
#include "AppConfig.hpp"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

UpdateChecker::UpdateChecker(const QString& currentVersion,
                             const QString& checkUrl,
                             QObject* parent)
    : QObject(parent)
    , currentVersion_(currentVersion)
    , checkUrl_(checkUrl) {
    manager_ = new QNetworkAccessManager(this);
    connect(manager_, &QNetworkAccessManager::finished,
            this, &UpdateChecker::onReplyFinished);
}

void UpdateChecker::checkForUpdates() {
    if (checking_) return;
    checking_ = true;

    QUrl url(checkUrl_);
    QNetworkRequest request{url};
    request.setHeader(QNetworkRequest::UserAgentHeader,
        QStringLiteral(APP_NAME "/%1").arg(currentVersion_));
    request.setTransferTimeout(10000);
    manager_->get(request);
}

bool UpdateChecker::isChecking() const {
    return checking_;
}

void UpdateChecker::onReplyFinished(QNetworkReply* reply) {
    checking_ = false;

    if (reply->error() != QNetworkReply::NoError) {
        emit checkError(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        emit checkError(QStringLiteral("Format de réponse invalide"));
        return;
    }

    QJsonObject obj = doc.object();
    QString latestVer = obj.value("version").toString();
    QString downloadUrl = obj.value("url").toString();
    QString releaseNotes = obj.value("notes").toString();

    if (latestVer.isEmpty()) {
        emit checkError(QStringLiteral("Version introuvable dans la réponse"));
        return;
    }

    QVersionNumber current = QVersionNumber::fromString(currentVersion_);
    QVersionNumber latest = QVersionNumber::fromString(latestVer);

    if (latest.isNull() || current.isNull()) {
        emit checkError(QStringLiteral("Impossible de comparer les versions"));
        return;
    }

    if (latest > current) {
        emit updateAvailable(latestVer, downloadUrl, releaseNotes);
    } else {
        emit upToDate();
    }
}
