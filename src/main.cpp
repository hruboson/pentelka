#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QLocale>
#include <QTranslator>

#include "canvas/item/canvasitem.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<CanvasItem>("Pentelka", 1, 0, "CanvasItem");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
