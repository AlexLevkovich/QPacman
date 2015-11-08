/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANINFOBROWSERDOCUMENT_H
#define PACMANINFOBROWSERDOCUMENT_H

#include <QTextDocument>
#include <QVariant>
#include <QUrl>

class PacmanItemModel;

class PacmanInfoBrowserDocument : public QTextDocument {
    Q_OBJECT
public:
    explicit PacmanInfoBrowserDocument(QObject *parent = 0);
    void setModel(PacmanItemModel * model) { m_model = model; }

protected:
    QVariant loadResource(int type,const QUrl & name);

private:
    PacmanItemModel * m_model;
};

#endif // PACMANINFOBROWSERDOCUMENT_H
