/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef FILESDOWNLOADDIALOG_H
#define FILESDOWNLOADDIALOG_H

#include <QDialog>
#include <QUrl>
#include "libqpacman_global.h"

class QCloseEvent;
class QKeyEvent;

class PacmanUpdatePackagesReader;
class PacmanProcessReader;

namespace Ui {
class FilesDownloadDialog;
}

class LIBQPACMANSHARED_EXPORT FilesDownloadDialog : public QDialog {
    Q_OBJECT

public:
    explicit FilesDownloadDialog(PacmanUpdatePackagesReader * installer,QWidget *parent = 0);
    ~FilesDownloadDialog();

private slots:
    void setValue(int percents);
    void setFileLength(int length);
    void finished(PacmanProcessReader * reader);

public slots:
    void setNewDownload(const QString & url);
#if QT_VERSION < 0x050000
    virtual void reject();
#endif
    
signals:
    void canceled();

protected:
    void closeEvent(QCloseEvent *e);
    void keyPressEvent(QKeyEvent *event);

private:
    Ui::FilesDownloadDialog *ui;
    int currFileNum;
    int m_files_count;
    bool doClose;
};

#endif // FILESDOWNLOADDIALOG_H
