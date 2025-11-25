#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QLoggingCategory>

#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QLoggingCategory::setFilterRules(QStringLiteral("qt.qml.warning=true\nqt.qml.info=true"));

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
