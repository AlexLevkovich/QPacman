/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef FILESLISTWIDGET_H
#define FILESLISTWIDGET_H

#include <QTreeWidget>
#include "alpmpackage.h"

class QLabel;

class FilesListWidget : public QTreeWidget {
    Q_OBJECT
public:
    explicit FilesListWidget(QWidget *parent = 0);

public slots:
    void fill(const AlpmPackage & pkg);
    void refill();

private slots:
    void newFileEntry(AlpmPackage::FileInfo & file);

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    bool event(QEvent *event);
    void mousePressEvent(QMouseEvent *event);

signals:
    void downloadRequested(const AlpmPackage & pkg);

private:
    void fill();

    QIcon folderIcon;
    QIcon fileIcon;
    bool m_do_refresh_picture;
    QPixmap m_refresh_pix;
    QPixmap m_refresh_shadow_pix;
    QPixmap pix;
    QPixmap shadow_pix;
    QPoint pt;
    QRect drawRect;
    QRect textRect;
    bool in_rect;
    AlpmPackage m_pkg;
    QMap<QString,QList<AlpmPackage::FileInfo> > m_files;
};

#endif // FILESLISTWIDGET_H
