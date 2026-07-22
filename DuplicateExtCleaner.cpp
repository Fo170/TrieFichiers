#include "DuplicateExtCleaner.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QDirIterator>
#include <QFileInfo>
#include <QRegularExpression>
#include <QFile>
#include <QDir>

DuplicateExtCleaner::DuplicateExtCleaner(QWidget* parent)
    : QWidget(parent)
{
    auto* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(8, 8, 8, 8);

    auto* path_layout = new QHBoxLayout;
    path_edit_ = new QLineEdit;
    path_edit_->setPlaceholderText(QStringLiteral("..."));
    btn_browse_ = new QPushButton(QStringLiteral("..."));
    btn_browse_->setFixedWidth(32);
    path_layout->addWidget(path_edit_);
    path_layout->addWidget(btn_browse_);
    main_layout->addLayout(path_layout);

    auto* ext_layout = new QHBoxLayout;
    ext_edit_ = new QLineEdit(QStringLiteral(".torrent"));
    btn_scan_ = new QPushButton(QStringLiteral("Rechercher"));
    ext_layout->addWidget(ext_edit_);
    ext_layout->addWidget(btn_scan_);
    main_layout->addLayout(ext_layout);

    count_label_ = new QLabel;
    count_label_->setVisible(false);
    main_layout->addWidget(count_label_);

    table_ = new QTableWidget(0, 3);
    table_->setHorizontalHeaderLabels({
        QStringLiteral("Fichier"),
        QStringLiteral("Nouveau nom"),
        QStringLiteral("Statut")
    });
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    main_layout->addWidget(table_);

    btn_fix_ = new QPushButton(QStringLiteral("Corriger"));
    btn_fix_->setEnabled(false);
    main_layout->addWidget(btn_fix_);

    connect(btn_browse_, &QPushButton::clicked,
            this, &DuplicateExtCleaner::browse);
    connect(btn_scan_, &QPushButton::clicked,
            this, &DuplicateExtCleaner::scan);
    connect(btn_fix_, &QPushButton::clicked,
            this, &DuplicateExtCleaner::fix);
}

void DuplicateExtCleaner::browse() {
    QString dir = QFileDialog::getExistingDirectory(
        this, QStringLiteral("..."), path_edit_->text());
    if (!dir.isEmpty())
        path_edit_->setText(dir);
}

void DuplicateExtCleaner::scan() {
    QString path = path_edit_->text().trimmed();
    if (path.isEmpty()) return;

    QString ext = ext_edit_->text().trimmed();
    if (ext.isEmpty()) return;
    if (!ext.startsWith('.'))
        ext.prepend('.');

    table_->setRowCount(0);
    btn_fix_->setEnabled(false);
    count_label_->setVisible(false);

    QString escaped = QRegularExpression::escape(ext);
    QRegularExpression re(QStringLiteral("^(.*?)(") + escaped + QStringLiteral("){2,}$"));
    if (!re.isValid()) return;

    QDirIterator it(path, QDir::Files, QDirIterator::Subdirectories);
    QSet<QString> seen;
    int count = 0;

    while (it.hasNext()) {
        it.next();
        QString file_path = it.filePath();
        QString file_name = it.fileName();

        QRegularExpressionMatch m = re.match(file_name);
        if (!m.hasMatch()) continue;

        QString new_name = m.captured(1) + ext;

        QString key = file_path.toLower();
        if (seen.contains(key)) continue;
        seen.insert(key);

        int row = table_->rowCount();
        table_->insertRow(row);

        QString rel_path = QDir(path).relativeFilePath(file_path);
        auto* item_path = new QTableWidgetItem(rel_path);
        item_path->setData(Qt::UserRole, file_path);
        table_->setItem(row, 0, item_path);
        table_->setItem(row, 1, new QTableWidgetItem(new_name));
        table_->setItem(row, 2, new QTableWidgetItem(QStringLiteral("...")));

        ++count;
    }

    if (count > 0) {
        count_label_->setText(
            QString::number(count) + QStringLiteral(" fichier(s) trouvé(s)"));
        count_label_->setVisible(true);
        btn_fix_->setEnabled(true);
    }

    emit status_message(
        QString::number(count) + QStringLiteral(" fichier(s) avec extension dupliquée"));
}

void DuplicateExtCleaner::fix() {
    int fixed = 0, errors = 0;

    for (int row = 0; row < table_->rowCount(); ++row) {
        auto* status_item = table_->item(row, 2);
        if (status_item && status_item->text() != QStringLiteral("..."))
            continue;

        QString old_path = table_->item(row, 0)->data(Qt::UserRole).toString();
        QString new_name = table_->item(row, 1)->text();
        QFileInfo fi(old_path);
        QString new_path = fi.absolutePath() + QStringLiteral("/") + new_name;

        if (QFile::exists(new_path)) {
            table_->item(row, 2)->setText(QStringLiteral("✗ existe déjà"));
            ++errors;
            continue;
        }

        if (QFile::rename(old_path, new_path)) {
            table_->item(row, 2)->setText(QStringLiteral("✓"));
            ++fixed;
        } else {
            table_->item(row, 2)->setText(QStringLiteral("✗ erreur"));
            ++errors;
        }
    }

    QString msg = QString::number(fixed) + QStringLiteral(" renommé(s)");
    if (errors > 0)
        msg += QStringLiteral(", ") + QString::number(errors) + QStringLiteral(" erreur(s)");
    emit status_message(msg);
}
