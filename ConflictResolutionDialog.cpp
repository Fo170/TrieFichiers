#include "ConflictResolutionDialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileInfo>
#include <QFrame>
#include <QDateTime>

ConflictResolutionDialog::ConflictResolutionDialog(
    const QString& sourcePath,
    const QString& destPath,
    QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Conflit de renommage");
    setMinimumWidth(500);
    setupUi(sourcePath, destPath);
}

QString ConflictResolutionDialog::formatSize(qint64 bytes) const {
    if (bytes < 1024)
        return QString::number(bytes) + " o";
    if (bytes < 1024 * 1024)
        return QString::number(bytes / 1024.0, 'f', 1) + " Ko";
    return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + " Mo";
}

void ConflictResolutionDialog::setupUi(const QString& sourcePath,
                                       const QString& destPath) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);

    auto* title = new QLabel(
        "Un fichier portant ce nom existe d\u00e9j\u00e0 dans le dossier de destination :");
    title->setStyleSheet("font-weight: bold; font-size: 13px;");
    mainLayout->addWidget(title);

    auto* grid = new QGridLayout;
    grid->setVerticalSpacing(6);
    grid->setHorizontalSpacing(16);

    int row = 0;

    auto addSection = [&](int r, const QString& label, const QString& path,
                          bool isDest, QLabel*& outFrame) {
        QFileInfo fi(path);

        auto* frame = new QLabel;
        frame->setFrameStyle(QFrame::Box | QFrame::Plain);
        frame->setLineWidth(2);
        frame->setMargin(8);
        auto* frameLayout = new QVBoxLayout(frame);

        auto* header = new QLabel(label);
        header->setStyleSheet(
            QStringLiteral("font-weight: bold; font-size: 12px; color: %1;")
                .arg(isDest ? "#c0392b" : "#2c3e50"));
        frameLayout->addWidget(header);

        auto addRow = [&](const QString& label, const QString& value) {
            auto* hl = new QHBoxLayout;
            auto* lbl = new QLabel(label);
            lbl->setStyleSheet("font-weight: bold; color: #555;");
            lbl->setFixedWidth(80);
            hl->addWidget(lbl);
            auto* val = new QLabel(value);
            val->setWordWrap(true);
            hl->addWidget(val, 1);
            frameLayout->addLayout(hl);
        };

        addRow("Nom :", fi.fileName());
        addRow("Chemin :", fi.absolutePath());
        addRow("Taille :", formatSize(fi.size()));
        addRow("Modifi\u00e9 le :",
               fi.lastModified().toString("dd/MM/yyyy hh:mm:ss"));

        if (isDest) {
            auto* warning = new QLabel(
                "\u26a0 Ce fichier sera \u00c9CRAS\u00c9 si vous "
                "choisissez \u00ab\u00c9craser\u00bb");
            warning->setStyleSheet(
                "color: #c0392b; font-weight: bold; font-style: italic; "
                "padding-top: 4px;");
            frameLayout->addWidget(warning);

            frame->setStyleSheet(
                "QLabel { border: 2px solid #e74c3c; border-radius: 6px; "
                "background-color: #fdf0ef; padding: 4px; }");
        } else {
            frame->setStyleSheet(
                "QLabel { border: 2px solid #bdc3c7; border-radius: 6px; "
                "background-color: #f9f9f9; padding: 4px; }");
        }

        grid->addWidget(frame, r, 0, 1, 2);
        return r + 1;
    };

    QLabel* dummy;
    row = addSection(row, "Fichier source (sera renomm\u00e9) :",
                     sourcePath, false, dummy);

    row = addSection(row, "Fichier existant (SERA \u00c9CRAS\u00c9) :",
                     destPath, true, dummy);

    mainLayout->addLayout(grid);
    mainLayout->addSpacing(12);

    auto* info = new QLabel(
        "Choisissez l'action \u00e0 effectuer :");
    info->setStyleSheet("font-weight: bold; font-size: 12px;");
    mainLayout->addWidget(info);

    auto* btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(6);

    btnOverwrite_ = new QPushButton("\u00c9craser ce fichier");
    btnSkip_ = new QPushButton("Ignorer ce fichier");
    btnOverwriteAll_ = new QPushButton("\u00c9craser tout");
    btnSkipAll_ = new QPushButton("Ignorer tout");
    btnCancel_ = new QPushButton("Annuler");

    btnLayout->addWidget(btnOverwrite_);
    btnLayout->addWidget(btnSkip_);
    btnLayout->addWidget(btnOverwriteAll_);
    btnLayout->addWidget(btnSkipAll_);
    btnLayout->addWidget(btnCancel_);
    mainLayout->addLayout(btnLayout);

    connect(btnOverwrite_, &QPushButton::clicked, this, [this]() {
        result_ = Overwrite;
        accept();
    });
    connect(btnSkip_, &QPushButton::clicked, this, [this]() {
        result_ = Skip;
        accept();
    });
    connect(btnOverwriteAll_, &QPushButton::clicked, this, [this]() {
        result_ = OverwriteAll;
        accept();
    });
    connect(btnSkipAll_, &QPushButton::clicked, this, [this]() {
        result_ = SkipAll;
        accept();
    });
    connect(btnCancel_, &QPushButton::clicked, this, [this]() {
        result_ = Cancel;
        reject();
    });
}
