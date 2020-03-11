/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "textimagehandler.h"
#include <QGuiApplication>
#include <QTextFormat>
#include <QPainter>
#include <QDebug>
#include <QFile>
#include <QPalette>
#include <QTextEdit>
#include <QThread>
#include <QScreen>

static int defaultDpiY() {
    if (QCoreApplication::instance()->testAttribute(Qt::AA_Use96Dpi)) return 96;
    if (const QScreen *screen = QGuiApplication::primaryScreen())
        return qRound(screen->logicalDotsPerInchY());
    //PI has not been initialised, or it is being initialised. Give a default dpi
    return 100;
}

static QString resolveFileName(QString fileName, QUrl *url) {
    if (url->isValid()) {
      if (url->scheme() == QLatin1String("qrc")) {
        fileName = fileName.right(fileName.length() - 3);
      }
      else if (url->scheme() == QLatin1String("file")) {
        fileName = url->toLocalFile();
      }
    }
    return fileName;
}

static QSize getPixmapSize(QTextDocument *doc, const QTextImageFormat &format);

static QPixmap _getPixmap(QTextDocument *doc, const QTextImageFormat &format) {
    QPixmap pm;
    QString name = format.name();
    if (name.startsWith(QLatin1String(":/"))) // auto-detect resources and convert them to url
        name.prepend(QLatin1String("qrc"));
    QUrl url = QUrl(name);
    name = resolveFileName(name, &url);
    const QVariant data = doc->resource(QTextDocument::ImageResource, url);
    if (data.type() == QVariant::Pixmap || data.type() == QVariant::Image) {
        pm = qvariant_cast<QPixmap>(data);
    } else if (data.type() == QVariant::ByteArray) {
        pm.loadFromData(data.toByteArray());
    }
    if (pm.isNull()) {
        // try direct loading
        QImage img;
        if (name.isEmpty() || !img.load(name))
            return QPixmap(QLatin1String(":/qt-project.org/styles/commonstyle/images/file-16.png"));
        pm = QPixmap::fromImage(img);
        doc->addResource(QTextDocument::ImageResource, url, pm);
    }

    return pm;
}

static QPixmap getPixmap(QTextDocument *doc, const QTextImageFormat &format) {
    return _getPixmap(doc,format).scaled(getPixmapSize(doc,format),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
}

static QSize getPixmapSize(QTextDocument *doc, const QTextImageFormat &format) {
    QPixmap pm;
    const bool hasWidth = format.hasProperty(QTextFormat::ImageWidth);
    const int width = qRound(format.width());
    const bool hasHeight = format.hasProperty(QTextFormat::ImageHeight);
    const int height = qRound(format.height());
    QSize size(width, height);
    if (!hasWidth || !hasHeight) {
        pm = _getPixmap(doc, format);
        const int pmWidth = pm.width() / pm.devicePixelRatio();
        const int pmHeight = pm.height() / pm.devicePixelRatio();
        if (!hasWidth) {
            if (!hasHeight) size.setWidth(pmWidth);
            else size.setWidth(qRound(height * (pmWidth / (qreal) pmHeight)));
        }
        if (!hasHeight) {
            if (!hasWidth) size.setHeight(pmHeight);
            else size.setHeight(qRound(width * (pmHeight / (qreal) pmWidth)));
        }
    }
    qreal scale = 1.0;
    QPaintDevice *pdev = doc->documentLayout()->paintDevice();
    if (pdev) {
        if (pm.isNull()) pm = _getPixmap(doc, format);
        if (!pm.isNull()) scale = qreal(pdev->logicalDpiY()) / qreal(defaultDpiY());
    }
    size *= scale;
    return size;
}

static QSize getImageSize(QTextDocument *doc, const QTextImageFormat &format);

static QImage _getImage(QTextDocument *doc, const QTextImageFormat &format) {
    QImage image;
    QString name = format.name();
    if (name.startsWith(QLatin1String(":/"))) name.prepend(QLatin1String("qrc"));
    QUrl url = QUrl(name);
    qreal sourcePixelRatio = 1.0;
    name = resolveFileName(name, &url);
    const QVariant data = doc->resource(QTextDocument::ImageResource, url);
    if (data.type() == QVariant::Image) {
        image = qvariant_cast<QImage>(data);
    } else if (data.type() == QVariant::ByteArray) {
        image.loadFromData(data.toByteArray());
    }
    if (image.isNull()) {
        if (name.isEmpty() || !image.load(name))
            return QImage(QLatin1String(":/qt-project.org/styles/commonstyle/images/file-16.png"));
        doc->addResource(QTextDocument::ImageResource, url, image);
    }
    if (sourcePixelRatio != 1.0) image.setDevicePixelRatio(sourcePixelRatio);

    return image.scaled(getImageSize(doc,format),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
}

static QImage getImage(QTextDocument *doc, const QTextImageFormat &format) {
    return _getImage(doc,format).scaled(getImageSize(doc,format),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
}

static QSize getImageSize(QTextDocument *doc, const QTextImageFormat &format) {
    QImage image;
    const bool hasWidth = format.hasProperty(QTextFormat::ImageWidth);
    const int width = qRound(format.width());
    const bool hasHeight = format.hasProperty(QTextFormat::ImageHeight);
    const int height = qRound(format.height());
    QSize size(width, height);
    if (!hasWidth || !hasHeight) {
        image = _getImage(doc, format);
        if (!hasWidth) size.setWidth(image.width() / image.devicePixelRatio());
        if (!hasHeight) size.setHeight(image.height() / image.devicePixelRatio());
    }
    qreal scale = 1.0;
    QPaintDevice *pdev = doc->documentLayout()->paintDevice();
    if (pdev) {
        if (image.isNull()) image = _getImage(doc, format);
        if (!image.isNull()) scale = qreal(pdev->logicalDpiY()) / qreal(defaultDpiY());
    }
    size *= scale;
    return size;
}

TextImageHandler::TextImageHandler(QTextEdit *parent) : QObject(parent->document()->documentLayout()) {
    QTextObjectInterface * handler = parent->document()->documentLayout()->handlerForObject(QTextFormat::ImageObject);
    parent->document()->documentLayout()->unregisterHandler(QTextFormat::ImageObject);
    parent->document()->documentLayout()->registerHandler(QTextFormat::ImageObject,this);
    delete handler;
}

QSizeF TextImageHandler::intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format) {
    Q_UNUSED(posInDocument)
    const QTextImageFormat imageFormat = format.toImageFormat();
    if (QCoreApplication::instance()->thread() != QThread::currentThread())
        return getImageSize(doc, imageFormat);
    return getPixmapSize(doc, imageFormat);
}

QImage TextImageHandler::image(QTextDocument *doc, const QTextImageFormat &imageFormat) {
    Q_ASSERT(doc != 0);
    return getImage(doc, imageFormat);
}

void TextImageHandler::drawObject(QPainter *p, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format) {
    Q_UNUSED(posInDocument)
        const QTextImageFormat imageFormat = format.toImageFormat();
    if (QCoreApplication::instance()->thread() != QThread::currentThread()) {
        QImage image = getImage(doc, imageFormat);
        p->drawImage(rect, image, image.rect());
    }
    else {
        QPixmap pixmap = getPixmap(doc, imageFormat);
        p->drawPixmap(rect, pixmap, pixmap.rect());
    }
}
