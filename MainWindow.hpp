#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include "Project.hpp"
#include "ComponentToolbox.hpp"

class QTreeWidgetItem;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void charger_projet();
    void sauvegarder_projet();
    void on_component_selected(const QVariantMap& data);
    void basculer_toolbox();

private:
    void create_menus();
    void create_toolbar();
    void update_title();

    Project project_;
    ComponentToolbox* toolbox_;
    QDockWidget* dock_toolbox_;
};

#endif
