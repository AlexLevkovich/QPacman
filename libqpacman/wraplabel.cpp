/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "wraplabel.h"

WrapLabel::WrapLabel(QWidget *parent) : QTextBrowser(parent) {
    init();
}

WrapLabel::WrapLabel(const QString & text, QWidget *parent) : QTextBrowser(parent) {
    init();
    setTextFormat(format,text);
}

void WrapLabel::init() {
    format = Qt::AutoText;
    setFrameStyle(QFrame::NoFrame);
    QPalette pal = palette();
    pal.setColor(QPalette::Base, Qt::transparent);
    setPalette(pal);
    setOpenExternalLinks(false);
    setOpenLinks(false);
    setLineWrapMode(QTextEdit::WidgetWidth);
    connect((QObject *)document()->documentLayout(),SIGNAL(documentSizeChanged(QSizeF)),this,SLOT(adjustMinimumSize(QSizeF)));
}

void WrapLabel::adjustMinimumSize(const QSizeF& size) {
    setMaximumHeight(size.height() + (2 * frameWidth()));
}

QString WrapLabel::text() const {
    if (real_format == Qt::RichText) return toHtml();
    return toPlainText();
}

void WrapLabel::setText(const QString & text) {
    setTextFormat(textFormat(),text);
}

Qt::TextFormat WrapLabel::textFormat() const {
    return format;
}

void WrapLabel::setTextFormat(Qt::TextFormat format, const QString & text_string) {
    QString text_str;
    if (!text_string.isEmpty()) text_str = text_string;
    else text_str = text();
    if ((format == Qt::AutoText) && Qt::mightBeRichText(text_str)) {
        real_format = Qt::RichText;
        label.setTextFormat(real_format);
        label.setText(text_str);
        QTextEdit::setHtml(text_str);
        return;
    }
    if (format == Qt::RichText) {
        real_format = Qt::RichText;
        label.setTextFormat(real_format);
        label.setText(text_str);
        QTextEdit::setHtml(text_str);
        return;
    }
    real_format = Qt::PlainText;
    label.setTextFormat(real_format);
    label.setText(text_str);
    QTextEdit::setText(text_str);
}

QSize WrapLabel::sizeHint() const {
    return QSize(label.sizeHint().width(),QTextEdit::sizeHint().height());
}
