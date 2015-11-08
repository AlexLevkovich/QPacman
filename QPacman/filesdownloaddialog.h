/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef FILESDOWNLOADDIALOG_H
#define FILESDOWNLOADDIALOG_H

#include <QDialog>
#include <QUrl>

class PacmanInstallPackagesReader;

namespace Ui {
class FilesDownloadDialog;
}

class FilesDownloadDialog : public QDialog {
    Q_OBJECT

public:
    explicit FilesDownloadDialog(PacmanInstallPackagesReader * installer,QWidget *parent = 0);
    ~FilesDownloadDialog();

private slots:
    void setValue(int percents);
    void setFileLength(int length);

public slots:
    void setNewDownload(const QString & url);

signals:
    void canceled();

private:
    Ui::FilesDownloadDialog *ui;
    int currFileNum;
    int m_files_count;

};

#endif // FILESDOWNLOADDIALOG_H
