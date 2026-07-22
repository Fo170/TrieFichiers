#include <QApplication>
#include <QFont>
#include <QIcon>
#include "AppConfig.hpp"
#include "MainWindow.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral(APP_NAME));

    QFont def = app.font();
    QFont emojiFont = def;
    emojiFont.setFamilies({"Noto Sans", "Noto Color Emoji"});
    app.setFont(emojiFont, "QMenu");
    app.setFont(emojiFont, "QMenuBar");
    app.setFont(emojiFont, "QToolBar");
    app.setFont(emojiFont, "QTreeWidget");

    QIcon icone;
    icone.addFile(":/ico/AZ.png", QSize(128, 128));
    icone.addFile(":/ico/app-256.png", QSize(256, 256));
    icone.addFile(":/ico/app-128.png", QSize(128, 128));
    icone.addFile(":/ico/app-64.png", QSize(64, 64));
    icone.addFile(":/ico/app-32.png", QSize(32, 32));
    app.setWindowIcon(icone);

    MainWindow fenetre;
    fenetre.setWindowIcon(icone);
    fenetre.show();

    return app.exec();
}
