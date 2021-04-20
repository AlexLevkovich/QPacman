/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef TEXTIMAGEHANDLER_H
#define TEXTIMAGEHANDLER_H

#include <QObject>
#include <QTextObjectInterface>

class QTextEdit;

class TextImageHandler : public QObject, public QTextObjectInterface {
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)
public:
    TextImageHandler(QTextEdit *parent);
    virtual QSizeF intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format) override;
    virtual void drawObject(QPainter *p, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format) override;
    QImage image(QTextDocument *doc, const QTextImageFormat &imageFormat);
};

#endif // TEXTIMAGEHANDLER_H
