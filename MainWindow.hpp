#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QCloseEvent>
#include <QStackedWidget>
#include "Project.hpp"
#include "ComponentToolbox.hpp"
#include "UpdateChecker.hpp"
#include "LangueManager.hpp"
#include "DuplicateExtCleaner.hpp"

class QAction;
class QDockWidget;
class QLabel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void nouveau_projet();
    void charger_projet();
    void sauvegarder_projet();
    void basculer_toolbox();
    void verifier_mise_a_jour();
    void on_update_available(const QString& version,
                             const QString& url,
                             const QString& notes);
    void on_up_to_date();
    void on_check_error(const QString& error);
    void changer_langue(const QString& langCode);
    void retranslateUi();
    void show_about();

private:
    void create_menus();
    void create_toolbar();
    void update_title();
    void update_project_page();
    void load_settings();
    void save_settings();
    void download_language(const QString& code);

    Project project_;
    ComponentToolbox* toolbox_;
    QDockWidget* dock_toolbox_;
    DuplicateExtCleaner* duplicate_cleaner_;
    UpdateChecker* update_checker_;
    LangueManager* langue_;
    QStackedWidget* stack_;
    QWidget* project_page_;
    QLabel* proj_name_label_;
    QLabel* proj_path_label_;

    QMenu* menu_fichier_;
    QMenu* menu_outils_;
    QMenu* menu_aide_;
    QMenu* menu_langue_;

    QAction* a_charger_ = nullptr;
    QAction* a_sauver_ = nullptr;
    QAction* a_quitter_ = nullptr;
    QAction* a_toolbox_ = nullptr;
    QAction* a_update_ = nullptr;
    QAction* a_about_ = nullptr;
    QAction* a_tb_charger_ = nullptr;
    QAction* a_tb_sauver_ = nullptr;
};

#endif
