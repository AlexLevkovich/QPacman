/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "fileslistwidget.h"
#include <QApplication>
#include <QDir>
#include <QFileInfo>

FilesListWidget::FilesListWidget(QWidget *parent) : QTreeWidget(parent) {
    folderIcon = QApplication::style()->standardIcon(QStyle::SP_DirIcon);
    fileIcon = QApplication::style()->standardIcon(QStyle::SP_FileIcon);

    setHeaderHidden(true);
}

void FilesListWidget::fill(const QStringList & files) {
    clear();

    QMap<QString,QStringList> m_files;
    for (int i=0;i<files.count();i++) {

        QFileInfo fi(files[i]);
        if (fi.dir().path() == ".") {
            m_files["/"].append(files[i]);
        }
        else {
            m_files[fi.dir().path()].append(fi.fileName());
        }
    }

    QMapIterator<QString, QStringList> i(m_files);
    while (i.hasNext()) {
        i.next();
        QTreeWidgetItem * item = new QTreeWidgetItem(this);
        item->setText(0,i.key());
        item->setIcon(0,folderIcon);
        QStringList values = i.value();
        for (int j=0;j<values.count();j++) {
            if (!values[j].isEmpty()) {
                QTreeWidgetItem * child_item = new QTreeWidgetItem(item);
                child_item->setText(0,values[j]);
                child_item->setIcon(0,fileIcon);
            }
        }
    }

    expandAll();
}


