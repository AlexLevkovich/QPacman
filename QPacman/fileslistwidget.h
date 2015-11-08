/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef FILESLISTWIDGET_H
#define FILESLISTWIDGET_H

#include <QTreeWidget>

class FilesListWidget : public QTreeWidget {
    Q_OBJECT
public:
    explicit FilesListWidget(QWidget *parent = 0);
public slots:
    void fill(const QStringList & files);

private:
    QIcon folderIcon;
    QIcon fileIcon;
};

#endif // FILESLISTWIDGET_H
