#include "CleanupDialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QCoreApplication>
#include <QFont>
#include <QTextCursor>
#include <QScrollBar>

CleanupDialog::CleanupDialog(const QStringList& initialPaths,
                             QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Nettoyage de dossier");
    resize(650, 550);

    auto* mainLayout = new QVBoxLayout(this);

    auto* pathLabel = new QLabel("Dossier(s) a nettoyer :");
    mainLayout->addWidget(pathLabel);

    pathList_ = new QListWidget;
    mainLayout->addWidget(pathList_);

    auto* btnLayout = new QHBoxLayout;
    auto* btnAdd = new QPushButton("Ajouter");
    auto* btnRemove = new QPushButton("Supprimer");
    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnRemove);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    chkEmptyFiles_ = new QCheckBox("Fichiers vides (0 octet)");
    chkEmptyFiles_->setChecked(true);
    mainLayout->addWidget(chkEmptyFiles_);

    chkThumbsDb_ = new QCheckBox("Thumbs.db");
    chkThumbsDb_->setChecked(true);
    mainLayout->addWidget(chkThumbsDb_);

    chkEmptyDirs_ = new QCheckBox("Dossiers vides");
    chkEmptyDirs_->setChecked(true);
    mainLayout->addWidget(chkEmptyDirs_);

    auto* actionLayout = new QHBoxLayout;
    btnAnalyze_ = new QPushButton("Analyser");
    btnClean_ = new QPushButton("Nettoyer");
    btnClean_->setEnabled(false);
    btnClose_ = new QPushButton("Fermer");
    actionLayout->addWidget(btnAnalyze_);
    actionLayout->addWidget(btnClean_);
    actionLayout->addStretch();
    actionLayout->addWidget(btnClose_);
    mainLayout->addLayout(actionLayout);

    logEdit_ = new QTextEdit;
    logEdit_->setReadOnly(true);
    logEdit_->setFont(QFont("Courier", 9));
    mainLayout->addWidget(logEdit_);

    summaryLabel_ = new QLabel;
    mainLayout->addWidget(summaryLabel_);

    for (const QString& p : initialPaths)
        pathList_->addItem(p);

    cleaner_ = new FolderCleaner(this);
    connect(cleaner_, &FolderCleaner::analysisProgress,
            this, &CleanupDialog::onAnalysisProgress);
    connect(cleaner_, &FolderCleaner::analysisFinished,
            this, &CleanupDialog::onAnalysisFinished);
    connect(cleaner_, &FolderCleaner::fileDeleted,
            this, &CleanupDialog::onFileDeleted);
    connect(cleaner_, &FolderCleaner::finished,
            this, &CleanupDialog::onCleaningFinished);

    connect(btnAdd, &QPushButton::clicked, this, &CleanupDialog::addPaths);
    connect(btnRemove, &QPushButton::clicked, this, &CleanupDialog::removeSelected);
    connect(btnAnalyze_, &QPushButton::clicked, this, &CleanupDialog::runAnalyze);
    connect(btnClean_, &QPushButton::clicked, this, &CleanupDialog::runClean);
    connect(btnClose_, &QPushButton::clicked, this, &QDialog::accept);
}

void CleanupDialog::addPaths() {
    QString dir = QFileDialog::getExistingDirectory(
        this, "Selectionner un dossier");
    if (!dir.isEmpty())
        pathList_->addItem(dir);
}

void CleanupDialog::removeSelected() {
    for (auto* item : pathList_->selectedItems())
        delete item;
}

void CleanupDialog::setButtonsEnabled(bool enabled) {
    btnAnalyze_->setEnabled(enabled);
    btnClean_->setEnabled(enabled && analysisDone_);
    pathList_->setEnabled(enabled);
    chkEmptyFiles_->setEnabled(enabled);
    chkThumbsDb_->setEnabled(enabled);
    chkEmptyDirs_->setEnabled(enabled);
}

void CleanupDialog::runAnalyze() {
    if (running_) return;

    if (pathList_->count() == 0) {
        QMessageBox::warning(this, "Attention",
                             "Ajoutez au moins un dossier a nettoyer.");
        return;
    }

    if (!chkEmptyFiles_->isChecked() &&
        !chkThumbsDb_->isChecked() &&
        !chkEmptyDirs_->isChecked()) {
        QMessageBox::warning(this, "Attention",
                             "Cochez au moins une operation a analyser.");
        return;
    }

    running_ = true;
    analysisDone_ = false;
    setButtonsEnabled(false);
    logEdit_->clear();
    summaryLabel_->clear();
    logEdit_->append("Analyse en cours...");

    QStringList paths;
    for (int i = 0; i < pathList_->count(); ++i)
        paths << pathList_->item(i)->text();

    cleaner_->analyze(paths,
                      chkEmptyFiles_->isChecked(),
                      chkThumbsDb_->isChecked(),
                      chkEmptyDirs_->isChecked());
}

void CleanupDialog::onAnalysisProgress(const QString& message) {
    logEdit_->append(message);
    logEdit_->moveCursor(QTextCursor::End);
    logEdit_->verticalScrollBar()->setValue(
        logEdit_->verticalScrollBar()->maximum());
    logEdit_->repaint();
    QCoreApplication::processEvents();
}

void CleanupDialog::onAnalysisFinished(int emptyFiles, int thumbsDb, int emptyDirs) {
    running_ = false;
    analysisDone_ = true;

    lastEmpty_ = emptyFiles;
    lastThumbs_ = thumbsDb;
    lastDirs_ = emptyDirs;

    logEdit_->clear();

    if (chkEmptyFiles_->isChecked())
        logEdit_->append(QString("  Fichiers vides (0 octet) : %1").arg(emptyFiles));
    if (chkThumbsDb_->isChecked())
        logEdit_->append(QString("  Thumbs.db              : %1").arg(thumbsDb));
    if (chkEmptyDirs_->isChecked())
        logEdit_->append(QString("  Dossiers vides          : %1").arg(emptyDirs));

    bool anyFound = (emptyFiles > 0 || thumbsDb > 0 || emptyDirs > 0);

    if (anyFound) {
        logEdit_->append("\nAppuyez sur 'Nettoyer' pour supprimer ces elements.");
        btnClean_->setEnabled(true);
    } else {
        logEdit_->append("\nAucun element a supprimer trouve.");
    }

    setButtonsEnabled(true);
}

void CleanupDialog::runClean() {
    if (running_ || !analysisDone_) return;

    if (lastEmpty_ == 0 && lastThumbs_ == 0 && lastDirs_ == 0) {
        QMessageBox::information(this, "Information",
                                 "L'analyse n'a rien trouve a nettoyer.");
        return;
    }

    QStringList paths;
    for (int i = 0; i < pathList_->count(); ++i)
        paths << pathList_->item(i)->text();

    running_ = true;
    analysisDone_ = false;
    setButtonsEnabled(false);
    logEdit_->clear();
    logEdit_->append("Nettoyage en cours...");

    cleaner_->clean(paths,
                    chkEmptyFiles_->isChecked(),
                    chkThumbsDb_->isChecked(),
                    chkEmptyDirs_->isChecked());
}

void CleanupDialog::onFileDeleted(const QString& path,
                                   const QString& operation) {
    QString op;
    if (operation == "empty_file") op = "VIDE";
    else if (operation == "thumbs_db") op = "THUMBS";
    else op = "DOSSIER";
    logEdit_->append(QString("[%1] %2").arg(op, path));
    QCoreApplication::processEvents();
}

void CleanupDialog::onCleaningFinished(int emptyFiles, int thumbsDb, int emptyDirs) {
    running_ = false;
    analysisDone_ = false;

    QStringList parts;
    if (emptyFiles > 0)
        parts << QString("%1 fichier(s) vide(s)").arg(emptyFiles);
    if (thumbsDb > 0)
        parts << QString("%1 Thumbs.db").arg(thumbsDb);
    if (emptyDirs > 0)
        parts << QString("%1 dossier(s) vide(s)").arg(emptyDirs);

    QString summary;
    if (parts.isEmpty())
        summary = "Rien n'a ete supprime.";
    else
        summary = "Supprime : " + parts.join(", ") + ".";

    summaryLabel_->setText(summary);
    logEdit_->append("\n=== " + summary + " ===");

    setButtonsEnabled(true);
}
