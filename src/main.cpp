#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtLogging>
#include <QQmlContext>

#include <QLocale>
#include <QTranslator>

#include "painter/painter.hpp"
#include "painter/imageprovider.hpp"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    if (QString(context.category) == "qml") { // filter only messages created by app qml
        QTextStream(stdout) << msg << Qt::endl;
    }
}

int main(int argc, char *argv[])
{

	#ifdef Q_OS_WIN // windows debugging logs
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
	#endif

    qInstallMessageHandler(messageHandler);

    QGuiApplication app(argc, argv);
	QQmlApplicationEngine engine;

	Painter painter;
    engine.rootContext()->setContextProperty("painter", &painter); // expose painter to qml
	engine.addImageProvider("painter", new PainterImageProvider(&painter));

    engine.load(QUrl(QStringLiteral("qrc:qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

	// Run app
    return app.exec();
}
