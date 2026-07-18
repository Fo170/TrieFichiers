#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QCloseEvent>
#include "Project.hpp"
#include "ComponentToolbox.hpp"
#include "UpdateChecker.hpp"
#include "LangueManager.hpp"

class QAction;
class QLabel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
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
    void load_settings();
    void save_settings();
    void download_language(const QString& code);

    Project project_;
    ComponentToolbox* toolbox_;
    QDockWidget* dock_toolbox_;
    UpdateChecker* update_checker_;
    LangueManager* langue_;
    QLabel* central_label_;
    QLabel* dock_title_;

    QMenu* menu_fichier_;
    QMenu* menu_outils_;
    QMenu* menu_aide_;
    QMenu* menu_langue_;

    QAction* a_charger_;
    QAction* a_sauver_;
    QAction* a_quitter_;
    QAction* a_toolbox_;
    QAction* a_update_;
    QAction* a_about_;
    QAction* a_tb_charger_;
    QAction* a_tb_sauver_;
};

#endif
