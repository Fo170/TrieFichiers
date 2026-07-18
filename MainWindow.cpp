#include "MainWindow.hpp"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QDockWidget>
#include <QFileInfo>
#include <QLabel>
#include <QDesktopServices>
#include <QUrl>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Application Vide");
    resize(1000, 650);

    toolbox_ = new ComponentToolbox;
    dock_toolbox_ = new QDockWidget("Boîte à outils", this);
    dock_toolbox_->setWidget(toolbox_);
    dock_toolbox_->setMinimumWidth(180);
    dock_toolbox_->setMaximumWidth(260);
    addDockWidget(Qt::LeftDockWidgetArea, dock_toolbox_);

    auto* central = new QLabel("Ouvrez ou créez un projet pour commencer");
    central->setAlignment(Qt::AlignCenter);
    central->setStyleSheet("color: #888; font-size: 14px;");
    setCentralWidget(central);

    connect(toolbox_, &ComponentToolbox::component_selected,
            this, &MainWindow::on_component_selected);

    update_checker_ = new UpdateChecker(
        QStringLiteral(APP_VERSION),
        QStringLiteral("https://raw.githubusercontent.com/Fo170/ApplicationVide/main/version.json"),
        this);
    connect(update_checker_, &UpdateChecker::updateAvailable,
            this, &MainWindow::on_update_available);
    connect(update_checker_, &UpdateChecker::upToDate,
            this, &MainWindow::on_up_to_date);
    connect(update_checker_, &UpdateChecker::checkError,
            this, &MainWindow::on_check_error);

    create_menus();
    create_toolbar();
    statusBar()->showMessage("Prêt");

    update_checker_->checkForUpdates();
}

void MainWindow::create_menus() {
    auto* mf = menuBar()->addMenu("&Fichier");

    auto* a_charger = mf->addAction("📂 Charger un projet");
    a_charger->setShortcut(QKeySequence::Open);
    connect(a_charger, &QAction::triggered, this, &MainWindow::charger_projet);

    auto* a_sauver = mf->addAction("💾 Sauvegarder le projet");
    a_sauver->setShortcut(QKeySequence::Save);
    connect(a_sauver, &QAction::triggered, this, &MainWindow::sauvegarder_projet);

    mf->addSeparator();

    auto* a_quitter = mf->addAction("❌ Quitter");
    a_quitter->setShortcut(QKeySequence::Quit);
    connect(a_quitter, &QAction::triggered, this, &QWidget::close);

    auto* mo = menuBar()->addMenu("&Outils");

    auto* a_toolbox = mo->addAction("🧰 Boîte à outils");
    a_toolbox->setCheckable(true);
    a_toolbox->setChecked(true);
    connect(a_toolbox, &QAction::triggered, this, &MainWindow::basculer_toolbox);

    auto* ma = menuBar()->addMenu("&?");

    auto* a_update = ma->addAction("🔄 Vérifier les mises à jour");
    connect(a_update, &QAction::triggered, this, &MainWindow::verifier_mise_a_jour);

    ma->addSeparator();

    auto* a_about = ma->addAction("ℹ️ À propos");
    connect(a_about, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "À propos",
            QString(
                "<h3>ApplicationVide</h3>"
                "<p>Version %1</p>"
                "<p>Application Qt6 multi-plateforme.</p>"
                "<p><a href='https://github.com/Fo170/ApplicationVide'>"
                "https://github.com/Fo170/ApplicationVide</a></p>")
                .arg(QStringLiteral(APP_VERSION)));
    });
}

void MainWindow::create_toolbar() {
    auto* tb = addToolBar("Principale");
    tb->setIconSize(QSize(16, 16));

    auto* a_tb_charger = tb->addAction("📂 Charger");
    connect(a_tb_charger, &QAction::triggered, this, &MainWindow::charger_projet);

    auto* a_tb_sauver = tb->addAction("💾 Sauvegarder");
    connect(a_tb_sauver, &QAction::triggered, this, &MainWindow::sauvegarder_projet);
}

void MainWindow::charger_projet() {
    QString p = QFileDialog::getOpenFileName(
        this, "Charger un projet", QString(),
        "Projet JSON (*.json);;Tous (*)");
    if (p.isEmpty()) return;

    if (!project_.load(p)) {
        QMessageBox::warning(this, "Erreur",
            "Impossible de charger le fichier projet.");
        return;
    }

    update_title();
    statusBar()->showMessage("Projet chargé : " + p);
}

void MainWindow::sauvegarder_projet() {
    QString p = QFileDialog::getSaveFileName(
        this, "Sauvegarder le projet", QString(),
        "Projet JSON (*.json);;Tous (*)");
    if (p.isEmpty()) return;

    if (!project_.save(p)) {
        QMessageBox::warning(this, "Erreur",
            "Impossible de sauvegarder le fichier projet.");
        return;
    }

    update_title();
    statusBar()->showMessage("Projet sauvegardé : " + p);
}

void MainWindow::on_component_selected(const QVariantMap& data) {
    statusBar()->showMessage(
        QString("Composant sélectionné : %1 (type: %2)")
            .arg(data.value("nom").toString())
            .arg(data.value("type").toString()));
}

void MainWindow::basculer_toolbox() {
    dock_toolbox_->setVisible(!dock_toolbox_->isVisible());
}

void MainWindow::verifier_mise_a_jour() {
    if (update_checker_->isChecking()) {
        statusBar()->showMessage("Vérification des mises à jour en cours...");
        return;
    }
    statusBar()->showMessage("Vérification des mises à jour...");
    update_checker_->checkForUpdates();
}

void MainWindow::on_update_available(const QString& version,
                                      const QString& url,
                                      const QString& notes) {
    QString msg = QString(
        "Une nouvelle version est disponible : %1\n\n"
        "Version actuelle : %2\n\n"
        "%3\n\n"
        "Voulez-vous télécharger la mise à jour ?")
        .arg(version, QStringLiteral(APP_VERSION), notes);

    auto btn = QMessageBox::question(this, "Mise à jour disponible",
        msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (btn == QMessageBox::Yes && !url.isEmpty())
        QDesktopServices::openUrl(QUrl(url));

    statusBar()->showMessage(
        QString("Mise à jour %1 disponible").arg(version));
}

void MainWindow::on_up_to_date() {
    statusBar()->showMessage(
        QString("Application à jour (v%1)").arg(QStringLiteral(APP_VERSION)));
}

void MainWindow::on_check_error(const QString& error) {
    statusBar()->showMessage("Vérification de mise à jour échouée : " + error);
}

void MainWindow::update_title() {
    QString titre = "Application Vide";
    if (!project_.file_path().isEmpty())
        titre = QFileInfo(project_.file_path()).fileName() + " — " + titre;
    setWindowTitle(titre);
}
