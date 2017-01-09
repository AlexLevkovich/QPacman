/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanInfoBrowser_H
#define PacmanInfoBrowser_H

#include <QTextBrowser>

class QTextDocument;
class PacmanItemModel;

class PacmanInfoBrowser : public QTextBrowser {
    Q_OBJECT
public:
    explicit PacmanInfoBrowser(QWidget *parent = 0);
    void setModel(PacmanItemModel * model);
    void clearImageCache();

protected:
    void setSource(const QUrl & name);

protected slots:
    void showContextMenu(const QPoint &pt);
    void openUrl(const QUrl & name);

signals:
    void groupUrlSelected(const QString &);
    void packageUrlSelected(const QString &);
    void reasonUrlSelected(const QString &);

private:
    PacmanItemModel * m_model;
};

#endif // PacmanInfoBrowser_H
