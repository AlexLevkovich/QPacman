/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef TEXTEDITHELPER_H
#define TEXTEDITHELPER_H

#include <QTextCursor>
#include <QPalette>
#include <QFont>
#include <QUrl>

class QTextTable;

class TextEditHelper {
public:
    TextEditHelper();

protected:
    void insertText(const QTextCursor & cursor,const QString & text);
    void insertText(const QTextCursor & cursor,const QString & text,QPalette::ColorRole color,QFont::Weight weight = QFont::Normal);
    void insertText(const QTextCursor & cursor,const QString & text,const QColor & color,QFont::Weight weight = QFont::Normal);
    void insertText(const QTextCursor & cursor,const QString & text,QFont::Weight weight);
    void insertLink(const QTextCursor & cursor,const QString & url,const QString & text = QString());
    void insertLink(const QTextCursor & cursor,const QString & url,const QString & text,QFont::Weight weight);
    void insertImage(const QTextCursor & cursor,const QString & name,const QSize & size = QSize(),const QUrl & url = QUrl()) const;
    void insertImage(const QTextCursor & cursor,const QImage & image,const QString & name,const QSize & size = QSize());
    QTextTable * insertTable(const QTextCursor & cursor,int rows,int columns,qreal border,Qt::Alignment aligment = Qt::AlignLeft,bool hasHeader = false);
    void setupTableHeader(QTextTable * table,const QColor & background,const QColor & foreground);
    void setTableRowColors(QTextTable * table,const QColor & evenBackground,const QColor & oddBackground);
    void setCellText(QTextTable * table,int row,int column,const QString & text,const QColor & background = QColor(),const QColor & foreground = QColor(),bool is_bold = false);
    void setCellLink(QTextTable * table,int row,int column,const QString & url,const QString & text);
};

#endif // TEXTEDITHELPER_H
