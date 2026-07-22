#include "ComponentToolbox.hpp"
#include <QVBoxLayout>
#include <QLabel>
#include <QTreeWidgetItem>
#include <QHeaderView>

ComponentToolbox::ComponentToolbox(QWidget* parent)
    : QWidget(parent) {
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(2, 2, 2, 2);

    auto* titre = new QLabel("<b>Boîte à outils</b>");
    titre->setAlignment(Qt::AlignCenter);
    lay->addWidget(titre);

    tree_ = new QTreeWidget;
    tree_->setHeaderHidden(true);
    tree_->setSelectionMode(QAbstractItemView::SingleSelection);
    tree_->setAnimated(true);
    tree_->setIndentation(12);
    lay->addWidget(tree_);

    populate_categories();

    connect(tree_, &QTreeWidget::itemClicked, this,
        [this](QTreeWidgetItem* item, int) {
            if (!item->parent()) return;
            QVariantMap data = item->data(0, Qt::UserRole).toMap();
            if (!data.isEmpty())
                emit component_selected(data);
        });
}

void ComponentToolbox::populate_categories() {
    auto groupe = [&](const QString& nom) -> QTreeWidgetItem* {
        auto* g = new QTreeWidgetItem(tree_, {nom});
        g->setExpanded(true);
        return g;
    };

    auto item = [&](QTreeWidgetItem* parent, const QString& nom,
                     const QVariantMap& data) {
        auto* i = new QTreeWidgetItem(parent, {nom});
        i->setData(0, Qt::UserRole, data);
        i->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    };

    auto* nettoyage = groupe("🧹 Nettoyage");
    item(nettoyage, "Supprimer fichiers vides",
         {{"type", "cleanup"}, {"tool", "clean_all"}});
    item(nettoyage, "Supprimer une extension",
         {{"type", "cleanup"}, {"tool", "strip_extension"}});
}
