/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef FILESLISTWIDGET_H
#define FILESLISTWIDGET_H

#include <QTreeWidget>
#include "alpmpackage.h"

class QLabel;
class ArchiveFileReaderLoop;
class WaitIndicator;
class ArchiveEntry;

class FilesListWidget : public QTreeWidget {
    Q_OBJECT
public:
    explicit FilesListWidget(QWidget *parent = 0);

public slots:
    void fill(AlpmPackage * pkg);
    void refill();

private slots:
    void newFileEntry(const QString & name,qint64 size,const QString & linkContents,const QDateTime & mdate,mode_t perms);
    void readerDestroyed();

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    bool event(QEvent *event);
    void mousePressEvent(QMouseEvent *event);

signals:
    void downloadRequested(AlpmPackage * pkg);

private:
    void fill();

    class FileEntry {
    public:
        FileEntry() {}
        FileEntry(const QString & name,qint64 size,const QString & linkContents,const QDateTime & mdate,mode_t perms);

        QString name() const { return m_name; }
        QString size() const { return m_size; }
        QString linkContents() const { return m_linkContents; }
        QString mdate() const { return m_mdate; }
        QString perm() const { return m_perms; }
    private:
        QString m_name;
        QString m_size;
        QString m_linkContents;
        QString m_mdate;
        QString m_perms;
    };

    QIcon folderIcon;
    QIcon fileIcon;
    Qt::FocusPolicy m_focusPolicy;
    bool m_do_refresh_picture;
    QPixmap m_refresh_pix;
    QPixmap m_refresh_shadow_pix;
    QPixmap pix;
    QPixmap shadow_pix;
    QPoint pt;
    QRect drawRect;
    QRect textRect;
    bool in_rect;
    AlpmPackage * m_pkg;
    ArchiveFileReaderLoop * reader;
    QMap<QString,QList<FileEntry> > m_files;
    WaitIndicator * wait_ind;
};

#endif // FILESLISTWIDGET_H
