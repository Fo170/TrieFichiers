#include "StripDialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QCoreApplication>
#include <QTextCursor>
#include <QScrollBar>

StripDialog::StripDialog(const QStringList& initialPaths,
                         QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Suppression d'extension");
    resize(650, 550);

    auto* mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(new QLabel("Dossier(s) a traiter :"));

    pathList_ = new QListWidget;
    mainLayout->addWidget(pathList_);

    auto* btnLayout = new QHBoxLayout;
    auto* btnAdd = new QPushButton("Ajouter");
    auto* btnRemove = new QPushButton("Supprimer");
    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnRemove);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    auto* extLayout = new QHBoxLayout;
    extLayout->addWidget(new QLabel("Extension a supprimer :"));
    extInput_ = new QLineEdit;
    extInput_->setPlaceholderText("ex: .az!, .bak, .old");
    extLayout->addWidget(extInput_);
    mainLayout->addLayout(extLayout);

    auto* actionLayout = new QHBoxLayout;
    btnAnalyze_ = new QPushButton("Analyser");
    btnApply_ = new QPushButton("Appliquer");
    btnApply_->setEnabled(false);
    btnClose_ = new QPushButton("Fermer");
    actionLayout->addWidget(btnAnalyze_);
    actionLayout->addWidget(btnApply_);
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
    connect(cleaner_, &FolderCleaner::stripProgress,
            this, &StripDialog::onStripProgress);
    connect(cleaner_, &FolderCleaner::stripAnalyzed,
            this, &StripDialog::onStripAnalyzed);
    connect(cleaner_, &FolderCleaner::stripFinished,
            this, &StripDialog::onStripFinished);

    connect(btnAdd, &QPushButton::clicked, this, &StripDialog::addPaths);
    connect(btnRemove, &QPushButton::clicked, this, &StripDialog::removeSelected);
    connect(btnAnalyze_, &QPushButton::clicked, this, &StripDialog::runAnalyze);
    connect(btnApply_, &QPushButton::clicked, this, &StripDialog::runApply);
    connect(btnClose_, &QPushButton::clicked, this, &QDialog::accept);
}

void StripDialog::addPaths() {
    QString dir = QFileDialog::getExistingDirectory(
        this, "Selectionner un dossier");
    if (!dir.isEmpty())
        pathList_->addItem(dir);
}

void StripDialog::removeSelected() {
    for (auto* item : pathList_->selectedItems())
        delete item;
}

void StripDialog::setButtonsEnabled(bool enabled) {
    btnAnalyze_->setEnabled(enabled);
    btnApply_->setEnabled(enabled && analysisDone_);
    pathList_->setEnabled(enabled);
    extInput_->setEnabled(enabled);
}

void StripDialog::runAnalyze() {
    if (running_) return;

    if (pathList_->count() == 0) {
        QMessageBox::warning(this, "Attention",
                             "Ajoutez au moins un dossier a analyser.");
        return;
    }

    QString ext = extInput_->text().trimmed();
    if (ext.isEmpty()) {
        QMessageBox::warning(this, "Attention",
                             "Entrez une extension a supprimer.");
        return;
    }
    if (!ext.startsWith('.'))
        ext.prepend('.');

    running_ = true;
    analysisDone_ = false;
    setButtonsEnabled(false);
    logEdit_->clear();
    summaryLabel_->clear();
    logEdit_->append(QString("Analyse pour l'extension %1 ...").arg(ext));

    QStringList paths;
    for (int i = 0; i < pathList_->count(); ++i)
        paths << pathList_->item(i)->text();

    extInput_->setText(ext);
    cleaner_->analyzeStrip(paths, ext);
}

void StripDialog::onStripProgress(const QString& message) {
    logEdit_->append(message);
    logEdit_->moveCursor(QTextCursor::End);
    logEdit_->verticalScrollBar()->setValue(
        logEdit_->verticalScrollBar()->maximum());
    logEdit_->repaint();
    QCoreApplication::processEvents();
}

void StripDialog::onStripAnalyzed(int count) {
    running_ = false;
    analysisDone_ = true;
    lastCount_ = count;

    logEdit_->append(QString("\nFichiers trouves : %1").arg(count));

    if (count > 0) {
        logEdit_->append("Appuyez sur 'Appliquer' pour renommer.");
        btnApply_->setEnabled(true);
    } else {
        logEdit_->append("Aucun fichier trouve avec cette extension.");
    }

    setButtonsEnabled(true);
}

void StripDialog::runApply() {
    if (running_ || !analysisDone_) return;

    if (lastCount_ == 0) {
        QMessageBox::information(this, "Information",
                                 "L'analyse n'a rien trouve a renommer.");
        return;
    }

    QString ext = extInput_->text().trimmed();
    QStringList paths;
    for (int i = 0; i < pathList_->count(); ++i)
        paths << pathList_->item(i)->text();

    running_ = true;
    analysisDone_ = false;
    setButtonsEnabled(false);
    logEdit_->clear();
    logEdit_->append("Renommage en cours...");

    cleaner_->applyStrip(paths, ext);
}

void StripDialog::onStripFinished(int renamed, int errors) {
    running_ = false;
    analysisDone_ = false;

    QStringList parts;
    if (renamed > 0)
        parts << QString("%1 fichier(s) renomme(s)").arg(renamed);
    if (errors > 0)
        parts << QString("%1 erreur(s)").arg(errors);

    QString summary;
    if (parts.isEmpty())
        summary = "Aucun fichier renomme.";
    else
        summary = parts.join(", ") + ".";

    summaryLabel_->setText(summary);
    logEdit_->append("\n=== " + summary + " ===");

    setButtonsEnabled(true);
}
