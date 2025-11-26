#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QLoggingCategory>
#include <QQmlContext>

#include <QLocale>
#include <QTranslator>

#include "painter/painter.hpp"
#include "painter/imageprovider.hpp"

int main(int argc, char *argv[])
{
    QLoggingCategory::setFilterRules(QStringLiteral("qt.qml.warning=true\nqt.qml.info=true"));

    QGuiApplication app(argc, argv);
	QQmlApplicationEngine engine;

	Painter painter;
    engine.rootContext()->setContextProperty("painter", &painter); // expose painter to qml
	engine.addImageProvider("painter", new PainterImageProvider(&painter));

    engine.load(QUrl(QStringLiteral("qrc:qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
