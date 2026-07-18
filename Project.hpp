#ifndef PROJECT_HPP
#define PROJECT_HPP

#include <QString>
#include <QJsonObject>
#include <QJsonArray>

class Project {
public:
    Project();

    bool load(const QString& path);
    bool save(const QString& path) const;

    void set_name(const QString& name);
    QString name() const;

    void add_component(const QJsonObject& component);
    QJsonArray components() const;

    void set_data(const QJsonObject& data);
    QJsonObject data() const;

    bool is_modified() const;
    void set_modified(bool modified);

    QString file_path() const;

private:
    QJsonObject data_;
    QString file_path_;
    bool modified_ = false;
};

#endif
