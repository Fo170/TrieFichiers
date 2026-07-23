#ifndef DUPLICATEEXTCLEANER_HPP
#define DUPLICATEEXTCLEANER_HPP

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "ConflictResolutionDialog.hpp"

class DuplicateExtCleaner : public QWidget {
    Q_OBJECT
public:
    explicit DuplicateExtCleaner(QWidget* parent = nullptr);

signals:
    void status_message(const QString& msg);

private slots:
    void browse();
    void scan();
    void fix();

private:
    QLineEdit* path_edit_;
    QLineEdit* ext_edit_;
    QPushButton* btn_browse_;
    QPushButton* btn_scan_;
    QPushButton* btn_fix_;
    QTableWidget* table_;
    QLabel* count_label_;
    ConflictResolutionDialog::Result global_choice_ = ConflictResolutionDialog::Skip;
};

#endif
