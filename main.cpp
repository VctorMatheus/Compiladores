#include "MainWindow.h"

#include <QApplication>
#include <QDirIterator>
#include <QFontDatabase>
#include <QStyleHints>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Estilo Fusion + esquema claro: a folha de estilo do papiro fica igual
    // independentemente do tema (claro/escuro) do Windows
    QApplication::setStyle(QStringLiteral("Fusion"));
    app.styleHints()->setColorScheme(Qt::ColorScheme::Light);

    // Registra as fontes embutidas (pasta fontes/, ver CMakeLists.txt)
    QDirIterator it(QStringLiteral(":/fontes"), {QStringLiteral("*.ttf")});
    while (it.hasNext())
        QFontDatabase::addApplicationFont(it.next());

    MainWindow janela;
    janela.show();
    return app.exec();
}
