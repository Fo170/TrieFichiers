#ifndef COMPONENTTOOLBOX_HPP
#define COMPONENTTOOLBOX_HPP

#include <QWidget>
#include <QTreeWidget>
#include <QVariantMap>

class ComponentToolbox : public QWidget {
    Q_OBJECT
public:
    explicit ComponentToolbox(QWidget* parent = nullptr);
    void selectProjectItem();

signals:
    void component_selected(const QVariantMap& data);

private:
    QTreeWidget* tree_;
    void populate_categories();
};

#endif
