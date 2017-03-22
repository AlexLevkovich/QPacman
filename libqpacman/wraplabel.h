/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef WRAPLABEL_H
#define WRAPLABEL_H

#include <QTextBrowser>
#include <QLabel>
#include "libqpacman_global.h"

class LIBQPACMANSHARED_EXPORT WrapLabel : public QTextBrowser {
    Q_OBJECT
public:
    explicit WrapLabel(QWidget *parent = 0);
    explicit WrapLabel(const QString & text, QWidget *parent = 0);
    void setText(const QString & text);
    QString text() const;
    Qt::TextFormat textFormat() const;
    void setTextFormat(Qt::TextFormat format, const QString & text_string = QString());
    QSize sizeHint() const;

private slots:
    void adjustMinimumSize(const QSizeF& size);

private:
    void init();
    void setHtml(const QString &) {}

    Qt::TextFormat format;
    Qt::TextFormat real_format;
    QLabel label;
};

#endif // WRAPLABEL_H
