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

    create_menus();
    create_toolbar();
    statusBar()->showMessage("Prêt");
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

void MainWindow::update_title() {
    QString titre = "Application Vide";
    if (!project_.file_path().isEmpty())
        titre = QFileInfo(project_.file_path()).fileName() + " — " + titre;
    setWindowTitle(titre);
}
