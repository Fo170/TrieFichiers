#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include "Project.hpp"
#include "ComponentToolbox.hpp"
#include "UpdateChecker.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void charger_projet();
    void sauvegarder_projet();
    void on_component_selected(const QVariantMap& data);
    void basculer_toolbox();
    void verifier_mise_a_jour();
    void on_update_available(const QString& version,
                             const QString& url,
                             const QString& notes);
    void on_up_to_date();
    void on_check_error(const QString& error);

private:
    void create_menus();
    void create_toolbar();
    void update_title();

    Project project_;
    ComponentToolbox* toolbox_;
    QDockWidget* dock_toolbox_;
    UpdateChecker* update_checker_;
};

#endif
