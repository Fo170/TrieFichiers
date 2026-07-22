#include "MainWindow.hpp"
#include "AppConfig.hpp"
#include "CleanupDialog.hpp"
#include "StripDialog.hpp"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QAbstractButton>
#include <QDockWidget>
#include <QFileInfo>
#include <QLabel>
#include <QDesktopServices>
#include <QUrl>
#include <QSettings>
#include <QDir>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    resize(1000, 650);

    langue_ = new LangueManager(
        QCoreApplication::applicationDirPath() + "/lang", this);

    load_settings();

    toolbox_ = new ComponentToolbox;
    dock_toolbox_ = new QDockWidget(this);
    dock_toolbox_->setWidget(toolbox_);
    dock_toolbox_->setMinimumWidth(180);
    dock_toolbox_->setMaximumWidth(260);
    addDockWidget(Qt::LeftDockWidgetArea, dock_toolbox_);

    central_label_ = new QLabel;
    central_label_->setAlignment(Qt::AlignCenter);
    central_label_->setStyleSheet("color: #888; font-size: 14px;");
    setCentralWidget(central_label_);

    connect(toolbox_, &ComponentToolbox::component_selected,
            this, [this](const QVariantMap& data) {
        if (data.value("type") == "cleanup") {
            if (data.value("tool") == "strip_extension") {
                StripDialog dlg({}, this);
                dlg.exec();
            } else {
                CleanupDialog dlg({}, this);
                dlg.exec();
            }
        } else {
            statusBar()->showMessage(
                QString(langue_->get("status.component_selected") + " : %1 (%2)")
                    .arg(data.value("nom").toString())
                    .arg(data.value("type").toString()));
        }
    });

    update_checker_ = new UpdateChecker(
        QStringLiteral(APP_VERSION),
        QStringLiteral(UPDATE_CHECK_URL), this);
    connect(update_checker_, &UpdateChecker::updateAvailable,
            this, &MainWindow::on_update_available);
    connect(update_checker_, &UpdateChecker::upToDate,
            this, &MainWindow::on_up_to_date);
    connect(update_checker_, &UpdateChecker::checkError,
            this, &MainWindow::on_check_error);

    create_menus();
    create_toolbar();
    retranslateUi();
    statusBar()->showMessage(langue_->get("status.ready"));

    update_checker_->checkForUpdates();
}

void MainWindow::create_menus() {
    menu_fichier_ = menuBar()->addMenu(QString());
    menu_outils_ = menuBar()->addMenu(QString());
    menu_langue_ = menuBar()->addMenu(QString());
    menu_aide_ = menuBar()->addMenu(QString());

    a_charger_ = menu_fichier_->addAction(QString());
    a_charger_->setShortcut(QKeySequence::Open);
    connect(a_charger_, &QAction::triggered, this, &MainWindow::charger_projet);

    a_sauver_ = menu_fichier_->addAction(QString());
    a_sauver_->setShortcut(QKeySequence::Save);
    connect(a_sauver_, &QAction::triggered, this, &MainWindow::sauvegarder_projet);

    menu_fichier_->addSeparator();

    a_quitter_ = menu_fichier_->addAction(QString());
    a_quitter_->setShortcut(QKeySequence::Quit);
    connect(a_quitter_, &QAction::triggered, this, &QWidget::close);

    a_toolbox_ = menu_outils_->addAction(QString());
    a_toolbox_->setCheckable(true);
    a_toolbox_->setChecked(dock_toolbox_->isVisible());
    connect(a_toolbox_, &QAction::triggered, this, &MainWindow::basculer_toolbox);

    connect(langue_, &LangueManager::languageChanged, this, &MainWindow::retranslateUi);
}

void MainWindow::create_toolbar() {
    auto* tb = addToolBar("main");
    tb->setIconSize(QSize(16, 16));

    a_tb_charger_ = tb->addAction(QString());
    connect(a_tb_charger_, &QAction::triggered, this, &MainWindow::charger_projet);

    a_tb_sauver_ = tb->addAction(QString());
    connect(a_tb_sauver_, &QAction::triggered, this, &MainWindow::sauvegarder_projet);
}

void MainWindow::retranslateUi() {
    menu_fichier_->setTitle(langue_->get("menu.file"));
    a_charger_->setText(langue_->get("menu.file.load"));
    a_sauver_->setText(langue_->get("menu.file.save"));
    a_quitter_->setText(langue_->get("menu.file.quit"));

    menu_outils_->setTitle(langue_->get("menu.tools"));
    a_toolbox_->setText(langue_->get("menu.tools.toolbox"));

    menu_langue_->setTitle(langue_->get("menu.language"));

    menu_aide_->setTitle(langue_->get("menu.help"));

    // Rebuild language submenu
    menu_langue_->clear();
    auto* group = new QActionGroup(this);
    QStringList codes = langue_->availableLanguages();
    QStringList names = langue_->languageDisplayNames();
    QString current = langue_->currentLanguage();

    for (int i = 0; i < codes.size(); ++i) {
        auto* a = menu_langue_->addAction(names.at(i));
        a->setCheckable(true);
        a->setChecked(codes.at(i) == current);
        a->setData(codes.at(i));
        group->addAction(a);
        connect(a, &QAction::triggered, this, [this, code = codes.at(i)]() {
            changer_langue(code);
        });
    }

    // Rebuild help menu (must be after langue menu to keep order)
    // Save existing update/about actions if they exist
    if (!a_update_) {
        menu_aide_->addSeparator();
        a_update_ = menu_aide_->addAction(QString());
        connect(a_update_, &QAction::triggered, this, &MainWindow::verifier_mise_a_jour);
        menu_aide_->addSeparator();
        a_about_ = menu_aide_->addAction(QString());
        connect(a_about_, &QAction::triggered, this, &MainWindow::show_about);
    }
    a_update_->setText(langue_->get("menu.help.check_update"));
    a_about_->setText(langue_->get("menu.help.about"));

    a_tb_charger_->setText(langue_->get("toolbar.load"));
    a_tb_sauver_->setText(langue_->get("toolbar.save"));

    dock_toolbox_->setWindowTitle(langue_->get("dock.toolbox"));
    central_label_->setText(langue_->get("central.placeholder"));

    setWindowTitle(langue_->get("app.name"));
}

void MainWindow::load_settings() {
    QSettings settings(QCoreApplication::applicationDirPath() + "/application.ini",
                       QSettings::IniFormat);

    QString lang = settings.value("langue", "").toString();
    if (lang.isEmpty())
        lang = LangueManager::detectSystemLanguage();

    if (!langue_->load(lang)) {
        download_language(lang);
        if (lang != "anglais")
            langue_->load("anglais");
    }

    QByteArray geo = settings.value("geometry").toByteArray();
    if (!geo.isEmpty())
        restoreGeometry(geo);

    save_settings();
}

void MainWindow::save_settings() {
    QSettings settings(QCoreApplication::applicationDirPath() + "/application.ini",
                       QSettings::IniFormat);
    settings.setValue("langue", langue_->currentLanguage());
    settings.setValue("geometry", saveGeometry());
}

void MainWindow::changer_langue(const QString& langCode) {
    if (!langue_->load(langCode))
        download_language(langCode);
    else
        save_settings();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    save_settings();
    QMainWindow::closeEvent(event);
}

void MainWindow::download_language(const QString& code) {
    QString url = QStringLiteral(LANG_BASE_URL) + code + ".txt";
    connect(langue_, &LangueManager::languageDownloaded, this,
        [this, code](const QString& langCode, bool success) {
            if (langCode != code) return;
            if (success && langue_->load(code)) {
                retranslateUi();
                save_settings();
                statusBar()->showMessage(langue_->get("status.ready"));
            }
        }, Qt::SingleShotConnection);
    langue_->downloadLanguage(code, url);
}

void MainWindow::show_about() {
    QString title = langue_->get("dialog.about.title");
    QString text = QString(langue_->get("dialog.about.text"))
        .arg(QStringLiteral(APP_NAME))
        .arg(QStringLiteral(APP_VERSION))
        + "<p><a href='" APP_HOMEPAGE_URL "'>"
          APP_HOMEPAGE_URL "</a></p>";
    QMessageBox::about(this, title, text);
}

void MainWindow::charger_projet() {
    QString p = QFileDialog::getOpenFileName(
        this, langue_->get("menu.file.load"), QString(),
        "Projet JSON (*.json);;Tous (*)");
    if (p.isEmpty()) return;

    if (!project_.load(p)) {
        QMessageBox::warning(this, langue_->get("menu.file.load"),
            langue_->get("dialog.error.load"));
        return;
    }

    update_title();
    statusBar()->showMessage(langue_->get("status.loaded") + " : " + p);
}

void MainWindow::sauvegarder_projet() {
    QString p = QFileDialog::getSaveFileName(
        this, langue_->get("menu.file.save"), QString(),
        "Projet JSON (*.json);;Tous (*)");
    if (p.isEmpty()) return;

    if (!project_.save(p)) {
        QMessageBox::warning(this, langue_->get("menu.file.save"),
            langue_->get("dialog.error.save"));
        return;
    }

    update_title();
    statusBar()->showMessage(langue_->get("status.saved") + " : " + p);
}

void MainWindow::basculer_toolbox() {
    dock_toolbox_->setVisible(!dock_toolbox_->isVisible());
    a_toolbox_->setChecked(dock_toolbox_->isVisible());
}

void MainWindow::verifier_mise_a_jour() {
    if (update_checker_->isChecking()) {
        statusBar()->showMessage(langue_->get("status.update_checking"));
        return;
    }
    statusBar()->showMessage(langue_->get("status.update_check"));
    update_checker_->checkForUpdates();
}

void MainWindow::on_update_available(const QString& version,
                                      const QString& url,
                                      const QString& notes) {
    QString msg = QString(langue_->get("dialog.update.message"))
        .arg(version, QStringLiteral(APP_VERSION), notes);

    QMessageBox mb(this);
    mb.setWindowTitle(langue_->get("dialog.update.title"));
    mb.setText(msg);
    mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    mb.button(QMessageBox::Yes)->setText(langue_->get("dialog.update.yes"));
    mb.button(QMessageBox::No)->setText(langue_->get("dialog.update.no"));
    mb.setDefaultButton(QMessageBox::Yes);

    if (mb.exec() == QMessageBox::Yes && !url.isEmpty())
        QDesktopServices::openUrl(QUrl(url));

    statusBar()->showMessage(
        QString(langue_->get("status.update_available")).arg(version));
}

void MainWindow::on_up_to_date() {
    statusBar()->showMessage(
        QString(langue_->get("status.up_to_date"))
            .arg(QStringLiteral(APP_VERSION)));
}

void MainWindow::on_check_error(const QString& error) {
    statusBar()->showMessage(
        langue_->get("status.update_failed") + " : " + error);
}

void MainWindow::update_title() {
    QString titre = langue_->get("app.name");
    if (!project_.file_path().isEmpty())
        titre = QFileInfo(project_.file_path()).fileName() + " — " + titre;
    setWindowTitle(titre);
}
