#pragma once

#include "tools.hpp"

#include <utility>

#include <QObject>
#include <QImage>
#include <QColor>

//TODO namespace pentelka

class Painter : public QObject {
    Q_OBJECT
public:
	explicit Painter(QWidget* parentWidget, QObject* parent = nullptr);

	// main drawing function
	Q_INVOKABLE void draw(const QPoint &from, const QPoint &to, const QColor &color, int width); // continuous function

	// tool selectors (should be called in qml)
	Q_INVOKABLE void selectBrush();
	Q_INVOKABLE void selectSpray();
	Q_INVOKABLE void selectEraser();
	Q_INVOKABLE void selectFill();
	Q_INVOKABLE void selectNewText();

	Q_INVOKABLE void setNewPatternOffset(int x, int y);
	Q_INVOKABLE void selectPatternNONE();
	Q_INVOKABLE void selectPatternCROSS();
	Q_INVOKABLE void selectPatternDIAGCROSS();
	Q_INVOKABLE void selectPatternDENSE();

	// drawing tools
	void setPixel(int x, int y, const QColor &color, int width);
	void setPixelPatterned(int x, int y, const QColor &color, int width, const Pattern* pattern);
	void drawLine(const QPoint &from, const QPoint &to, const QColor &color, int width, const Pattern* pattern = nullptr);
	void drawWuLine(const QPoint &from, const QPoint &to, const QColor &color, int width); // better line algorithm
	void sprayAt(const QPoint &at, const QColor &color, int width);
	void fillArea(const QPoint &at, const QColor &color, const Pattern* pattern = nullptr);

	// text tools
	Q_INVOKABLE void updateText(const QString &text, const QPoint &pos, const QColor &color, int fontSize);
	Q_INVOKABLE void commitText();

	// utilities
    Q_INVOKABLE bool loadImage(const QString &path);
    Q_INVOKABLE bool saveImage(const QString &path);
	Q_INVOKABLE void requestPrint();
	Q_INVOKABLE void resizeCanvas(int width, int height);
    void resizeBuffer(int width, int height);

    // expose the image buffer to QML
    Q_INVOKABLE QImage getBuffer() const;
	Q_INVOKABLE void setPreview(bool show);

	void clearBuffer(int width, int height);
	void clearBuffer(); // QML doesn't seem to be able to take function with optional parameters hence the overload
    void flush(); // emit bufferChanged once per frame

signals:
	void bufferChanged();
	void imageSizeChanged(int width, int height);

private:
    QWidget* parentWidget;

	QColor backgroundColor = Qt::white;

	// FIX snake to camel case
    QImage image_buffer; // canvas content stored here
	QImage image_text_buffer;

	QImage preview_buffer; // not included inthe complete image buffer
	bool showPreview;

    QTimer *updateTimer;
    bool pendingUpdate = false;

	// Tool to paint with
	TOOLS selectedTool = TOOLS::BRUSH;
	TOOLPATTERN selectedPattern = TOOLPATTERN::NONE;
	std::pair<int, int> selectedPatternOffset = {0, 0};


	// text tools
	QString currentText;
	QPoint textPos;
	int textFontSize;
	QColor textColor;
    bool caretVisible = true;
    QTimer *caretTimer;
};
