/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANINFOBROWSERDOCUMENT_H
#define PACMANINFOBROWSERDOCUMENT_H

#include <QTextDocument>
#include <QVariant>
#include <QUrl>
#include <QMap>
#include <QPixmap>

class PacmanItemModel;

class PacmanInfoBrowserDocument : public QTextDocument {
    Q_OBJECT
public:
    explicit PacmanInfoBrowserDocument(QObject *parent = 0);
    void setModel(PacmanItemModel * model) { m_model = model; }
    void clearImageCache();

protected:
    QVariant loadResource(int type,const QUrl & name);

private:
    PacmanItemModel * m_model;
    QMap<QUrl,QPixmap> img_cache;
};

#endif // PACMANINFOBROWSERDOCUMENT_H
