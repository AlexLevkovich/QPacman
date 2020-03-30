/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "textedithelper.h"
#include <QTextCursor>
#include <QTextTableFormat>
#include <QTextTable>
#include <QPalette>
#include <QApplication>
#include <QTextDocument>

TextEditHelper::TextEditHelper() {}

void TextEditHelper::insertText(const QTextCursor & cursor,const QString & text,QPalette::ColorRole color,QFont::Weight weight) {
    insertText(cursor,text,qApp->palette().color(color),weight);
}

void TextEditHelper::insertText(const QTextCursor & cursor,const QString & text,QFont::Weight weight) {
    insertText(cursor,text,qApp->palette().color(QPalette::WindowText),weight);
}

void TextEditHelper::insertText(const QTextCursor & cursor,const QString & text) {
    insertText(cursor,text,qApp->palette().color(QPalette::WindowText),QFont::Normal);
}

void TextEditHelper::insertText(const QTextCursor & cursor,const QString & text,const QColor & color,QFont::Weight weight) {
    QTextCharFormat txtfmt;
    txtfmt.setFontWeight(weight);
    txtfmt.setForeground(color);
    QTextCursor(cursor).insertText(text,txtfmt);
}

void TextEditHelper::insertLink(const QTextCursor & cursor,const QString & url,const QString & text) {
    insertLink(cursor,url,text,QFont::Normal);
}

void TextEditHelper::insertLink(const QTextCursor & cursor,const QString & url,const QString & text,QFont::Weight weight) {
    QTextCharFormat txtfmt;
    txtfmt.setAnchor(true);
    txtfmt.setFontWeight(weight);
    txtfmt.setForeground(qApp->palette().color(QPalette::Link));
    txtfmt.setAnchorHref(url);
    QTextCursor(cursor).insertText(text.isEmpty()?url:text,txtfmt);
}

void TextEditHelper::insertImage(const QTextCursor & cursor,const QString & name,const QSize & size,const QUrl & url) const {
    QTextImageFormat imageft;
    if (!url.isEmpty()) {
        imageft.setAnchor(true);
        imageft.setAnchorHref(url.toString());
    }
    imageft.setName(name);
    if (size.isValid()) {
        imageft.setWidth(size.width());
        imageft.setHeight(size.height());
    }
    QTextCursor(cursor).insertImage(imageft);
}

void TextEditHelper::insertImage(const QTextCursor & cursor,const QImage & image,const QString & name,const QSize & size) {
    cursor.document()->addResource(QTextDocument::ImageResource,QUrl(name),size.isValid()?image.scaled(size,Qt::IgnoreAspectRatio,Qt::SmoothTransformation):image);
    QTextCursor(cursor).insertImage(name);
}

QTextTable * TextEditHelper::insertTable(const QTextCursor & cursor,int rows,int columns,qreal border,Qt::Alignment aligment,bool hasHeader) {
    QTextTableFormat tableft;
    tableft.setAlignment(aligment);
    tableft.setBorderStyle(!border?QTextFrameFormat::BorderStyle_None:QTextFrameFormat::BorderStyle_Solid);
    tableft.setBorder(border);
    tableft.setCellPadding(2);
    tableft.setCellSpacing(-1);
    if (hasHeader) tableft.setHeaderRowCount(1);
    return QTextCursor(cursor).insertTable(rows,columns,tableft);
}

void TextEditHelper::setCellText(QTextTable * table,int row,int column,const QString & text,const QColor & background,const QColor & foreground,bool is_bold) {
    QTextTableCellFormat cellft;
    cellft.setVerticalAlignment(QTextCharFormat::AlignMiddle);
    if(foreground.isValid()) cellft.setForeground(foreground);
    if(background.isValid()) cellft.setBackground(background);
    QTextTableCell cell = table->cellAt(row,column);
    cell.setFormat(cellft);
    insertText(cell.firstCursorPosition(),text,is_bold?QFont::Bold:QFont::Normal);
}

void TextEditHelper::setCellLink(QTextTable * table,int row,int column,const QString & url,const QString & text) {
    insertLink(table->cellAt(row,column).firstCursorPosition(),url,text);
}

void TextEditHelper::setupTableHeader(QTextTable * table,const QColor & background,const QColor & foreground) {
    for (int col = 0;col < table->columns();col++) {
        QTextTableCell cell = table->cellAt(0,col);
        QTextCharFormat format = cell.format();
        format.setBackground(background);
        format.setForeground(foreground);
        cell.setFormat(format);
    }
}

void TextEditHelper::setTableRowColors(QTextTable * table,const QColor & evenBackground,const QColor & oddBackground) {
    for (int row = 1;row < table->rows();row++) {
        for (int col = 0;col < table->columns();col++) {
            QTextTableCell cell = table->cellAt(row,col);
            QTextCharFormat format = cell.format();
            format.setBackground(((row % 2) == 0)?evenBackground:oddBackground);
            cell.setFormat(format);
        }
    }
}
