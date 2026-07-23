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
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    resize(1000, 650);

    langue_ = new LangueManager(
        QCoreApplication::applicationDirPath() + "/lang", this);

    load_settings();

    // --- Left dock: toolbox ---
    toolbox_ = new ComponentToolbox;
    dock_toolbox_ = new QDockWidget(this);
    dock_toolbox_->setWidget(toolbox_);
    dock_toolbox_->setMinimumWidth(180);
    dock_toolbox_->setMaximumWidth(260);
    addDockWidget(Qt::LeftDockWidgetArea, dock_toolbox_);

    // --- Central area: stacked pages ---
    stack_ = new QStackedWidget;
    setCentralWidget(stack_);

    // Page 0: Project page
    project_page_ = new QWidget;
    auto* projLayout = new QVBoxLayout(project_page_);
    projLayout->setAlignment(Qt::AlignCenter);

    auto* proj_title = new QLabel;
    proj_title->setObjectName("proj_title");
    proj_title->setAlignment(Qt::AlignCenter);
    proj_title->setStyleSheet("font-size: 20px; font-weight: bold;");

    proj_name_label_ = new QLabel;
    proj_name_label_->setAlignment(Qt::AlignCenter);
    proj_name_label_->setStyleSheet("font-size: 14px; color: #555;");

    proj_path_label_ = new QLabel;
    proj_path_label_->setAlignment(Qt::AlignCenter);
    proj_path_label_->setStyleSheet("color: #888; font-style: italic;");
    proj_path_label_->setWordWrap(true);

    auto* btnLayout = new QHBoxLayout;
    btnLayout->setAlignment(Qt::AlignCenter);
    btnLayout->setSpacing(10);
    auto* btnNew = new QPushButton("Nouveau");
    auto* btnLoad = new QPushButton("Charger");
    auto* btnSave = new QPushButton("Sauvegarder");
    btnNew->setMinimumWidth(120);
    btnLoad->setMinimumWidth(120);
    btnSave->setMinimumWidth(120);
    btnLayout->addWidget(btnNew);
    btnLayout->addWidget(btnLoad);
    btnLayout->addWidget(btnSave);

    projLayout->addStretch();
    projLayout->addWidget(proj_title);
    projLayout->addSpacing(8);
    projLayout->addWidget(proj_name_label_);
    projLayout->addSpacing(4);
    projLayout->addWidget(proj_path_label_);
    projLayout->addSpacing(20);
    projLayout->addLayout(btnLayout);
    projLayout->addStretch();

    connect(btnNew, &QPushButton::clicked, this, &MainWindow::nouveau_projet);
    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::charger_projet);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::sauvegarder_projet);

    stack_->addWidget(project_page_);  // index 0

    // Page 1: Duplicate extension cleaner
    duplicate_cleaner_ = new DuplicateExtCleaner;
    stack_->addWidget(duplicate_cleaner_);  // index 1

    connect(duplicate_cleaner_, &DuplicateExtCleaner::status_message,
            this, [this](const QString& msg) {
        statusBar()->showMessage(msg);
    });

    // --- Toolbox selection -> page switching ---
    connect(toolbox_, &ComponentToolbox::component_selected,
            this, [this](const QVariantMap& data) {
        QString type = data.value("type").toString();
        if (type == "project") {
            stack_->setCurrentIndex(0);
        } else if (type == "cleanup") {
            QString tool = data.value("tool").toString();
            if (tool == "strip_extension") {
                StripDialog dlg({}, this);
                dlg.exec();
            } else if (tool == "dedup_extension") {
                stack_->setCurrentWidget(duplicate_cleaner_);
            } else {
                CleanupDialog dlg({}, this);
                dlg.exec();
            }
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

    // Select "Projet" in toolbox -> shows project page
    toolbox_->selectProjectItem();

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

    auto* proj_title = project_page_->findChild<QLabel*>("proj_title");
    if (proj_title)
        proj_title->setText(langue_->get("project.title"));

    if (!project_.file_path().isEmpty()) {
        proj_name_label_->setText(
            QStringLiteral("%1 : %2")
                .arg(langue_->get("project.name_label"), project_.name()));
        proj_path_label_->setText(
            QStringLiteral("%1 : %2")
                .arg(langue_->get("project.path_label"), project_.file_path()));
    } else {
        proj_name_label_->setText(langue_->get("project.no_project"));
        proj_path_label_->clear();
    }

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

void MainWindow::update_project_page() {
    if (!project_.file_path().isEmpty()) {
        proj_name_label_->setText(
            QStringLiteral("%1 : %2")
                .arg(langue_->get("project.name_label"), project_.name()));
        proj_path_label_->setText(
            QStringLiteral("%1 : %2")
                .arg(langue_->get("project.path_label"), project_.file_path()));
    } else {
        proj_name_label_->setText(langue_->get("project.no_project"));
        proj_path_label_->clear();
    }
}

void MainWindow::nouveau_projet() {
    // Reset project to defaults
    project_ = Project();
    project_.set_name(langue_->get("project.default_name"));
    update_project_page();
    update_title();
    statusBar()->showMessage(langue_->get("status.ready"));
    stack_->setCurrentIndex(0);
    toolbox_->selectProjectItem();
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

    update_project_page();
    update_title();
    statusBar()->showMessage(langue_->get("status.loaded") + " : " + p);

    // Switch to project page if not already there
    stack_->setCurrentIndex(0);
    toolbox_->selectProjectItem();
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

    update_project_page();
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
